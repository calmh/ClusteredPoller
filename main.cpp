#define STATISTICS
#define USE_MYSQL
#define MAXERRORSPERHOST 4

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <pthread.h>
#include "types.h"

#ifdef USE_MYSQL
#include <mysql++.h>
#endif

using namespace std;


// Global variables.
vector<QueryHost> hosts;
vector<ResultCache> cache;
RTGConf config;
unsigned thread_id = 0;

#ifdef STATISTICS
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
unsigned active_threads = 0;
unsigned stat_inserts = 0;
unsigned stat_queries = 0;
unsigned stat_iterations = 0;
#endif

map<string, ResultSet> query(QueryHost qh)
{
	map<string, ResultSet> rs;

	struct snmp_session session, *ss;
	struct snmp_pdu *pdu;
	struct snmp_pdu *response;
	oid anOID[MAX_OID_LEN];
	size_t anOID_len = MAX_OID_LEN;
	int status;
	void *sessp;

	snmp_sess_init(&session);
	session.peername = (char*) qh.host.c_str();
	session.version = SNMP_VERSION_2c;
	session.community = (u_char*) qh.community.c_str();
	session.community_len = qh.community.length();
	sessp = snmp_sess_open(&session);

	if (!sessp) {
		snmp_perror("ack");
		snmp_log(LOG_ERR, "something horrible happened!!!\n");
		return rs;
	}

	ss = snmp_sess_session(sessp);

	int errors = 0;
	list<QueryRow>::iterator it;
	for (it = qh.rows.begin(); it != qh.rows.end(); it++) {
		QueryRow row = *it;
		if (rs.find(row.table) == rs.end()) {
			ResultSet r(row.table);
			rs[row.table] = r;
		}
		pdu = snmp_pdu_create(SNMP_MSG_GET);
		read_objid(row.oid.c_str(), anOID, &anOID_len);
		snmp_add_null_var(pdu, anOID, anOID_len);

		status = snmp_sess_synch_response(sessp, pdu, &response);
		time_t response_time = time(NULL);

		if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
			struct variable_list *vars = response->variables;
			int got_value = 0;
			uint64_t value = 0;
			switch (vars->type) {
				case SNMP_NOSUCHOBJECT:
				case SNMP_NOSUCHINSTANCE:
					// Do nothing
				break;

				case ASN_INTEGER:
				case ASN_COUNTER:
				case ASN_GAUGE:
				case ASN_OPAQUE:
					// Regular integer
				value = *vars->val.integer;
				got_value = 1;
				break;

				case ASN_COUNTER64:
					// Get high and low 32 bits and shift them together
				value = (((uint64_t)(*vars->val.counter64).high) << 32) + (*vars->val.counter64).low;
				got_value = 1;
				break;

				default:
					// Notify unknown type
				pthread_mutex_lock(&global_lock);
				cerr << "SNMP get for " << qh.host << " OID " << row.oid
					<< " returned unknown variable type " << (unsigned) vars->type << endl;
				pthread_mutex_unlock(&global_lock);
			}

			// If we got a value, put it in the result set.
			if (got_value) {
				ResultRow r(row.id, value, 0, row.bits, response_time);
				rs[row.table].rows.push_back(r);
			}
		} else {
			pthread_mutex_lock(&global_lock);
			cerr << "SNMP get for " << qh.host << " OID " << row.oid << " failed." << endl;
			if (status == STAT_SUCCESS)
				cerr << "  Error in packet: " << snmp_errstring(response->errstat) << endl;
			else
				snmp_sess_perror("  Communication error: ", ss);
			pthread_mutex_unlock(&global_lock);

			errors++;
			if (errors > MAXERRORSPERHOST) {
				pthread_mutex_lock(&global_lock);
				cerr << "Too many errors for host " << qh.host << ", aborting." << endl;
				pthread_mutex_unlock(&global_lock);
				break;
			}
		}

		if (response)
			snmp_free_pdu(response);
	}

	snmp_sess_close(sessp);

	return rs;
}

void process_host(QueryHost &host, ResultCache &cache) {
#ifdef USE_MYSQL
	mysqlpp::Connection conn(true);
	conn.connect(config.database.c_str(), config.dbhost.c_str(), config.dbuser.c_str(), config.dbpass.c_str());
#endif

	// Query all values specified in the QueryHost and get back a list of ResultSets.
	// Each ResultSet represents one table in the database.
	map<string, ResultSet> results = query(host);

	// Iterate over all the ResultSets we got back.
	map<string, ResultSet>::iterator it;
	for (it = results.begin(); it != results.end(); it++) {
		ResultSet r = it->second;
		if (r.rows.size() > 0) {
			stringstream insert_query;
			stringstream query_values;
			insert_query << "INSERT INTO " << r.table << " (id, dtime, counter, rate) VALUES ";
			int inserted_rows = 0;

			// Iterate over all the ResultRows in this ResultSet
			list<ResultRow>::iterator ri;
			for (ri = r.rows.begin(); ri != r.rows.end(); ri++) {
				ResultRow row = *ri;
				pair<string, int> key = pair<string, int>(r.table, row.id);

				// If we have a previous measurement, calculate counter_diff and rate
				// and create an insert statement
				time_t prev_time = cache.times[key];
				if (prev_time != 0) {
					time_t time_diff = row.dtime - prev_time;
					uint64_t prev_counter = cache.counters[key];
					uint64_t counter_diff = row.counter - prev_counter;
					if (prev_counter > row.counter) {
						if (row.bits == 64)
							counter_diff -= (uint64_t)(-1);
						else
							counter_diff -= (uint32_t)(-1);
					}
					uint64_t rate = 0;
					if (row.bits == 0)
						rate = row.counter;
					else
						rate = counter_diff / time_diff;
					if (inserted_rows > 0)
						query_values << ", ";
					query_values << "(" << row.id << ", FROM_UNIXTIME(" << row.dtime << "), " << counter_diff << ", " << rate << ")";
					inserted_rows++;
				}

				// Update the cache for next iteration
				cache.counters[key] = row.counter;
				cache.times[key] = row.dtime;
			}

			// If we have at least one row to insert, push it to the database.
			if (inserted_rows > 0) {
#ifdef STATISTICS
				// Update statistics
				pthread_mutex_lock(&global_lock);
				stat_inserts += inserted_rows;
				stat_queries ++;
				pthread_mutex_unlock(&global_lock);
#endif

				insert_query << query_values.str();
#ifdef USE_MYSQL
				mysqlpp::Query q = conn.query(insert_query.str());
				q.exec();
#else
				cout << insert_query.str() << endl;
#endif
			}
		}
	}
}

void thread_loop() {
	unsigned stride = config.threads;
	pthread_mutex_lock(&global_lock);
	unsigned offset = thread_id++;
	// cerr << "Starting thread with offset " << offset << " and stride " << stride << "." << endl;
	pthread_mutex_unlock(&global_lock);
	unsigned iterations = 0;
	while (1) {
#ifdef STATISTICS
		// Mark ourself active
		pthread_mutex_lock(&global_lock);
		active_threads++;
		// cerr << "Active threads: " << active_threads << endl;
		pthread_mutex_unlock(&global_lock);
#endif

		time_t start = time(NULL);
		for (unsigned i = offset; i < hosts.size(); i += stride) {
			QueryHost host = hosts[i];
			process_host(host, cache[i]);
		}
		sleep(1);
		time_t end = time(NULL);
		time_t sleep_time = config.interval - (end - start);
		pthread_mutex_lock(&global_lock);
		// cerr << "Thread " << offset << " sleeping " << sleep_time << " seconds." << endl;

#ifdef STATISTICS
		// Mark ourself sleeping
		active_threads--;
		// cerr << "Remaining active threads: " << active_threads << endl;
		if (iterations < stat_iterations) {
			cerr << "Thread " << offset << " is behind schedule!" << endl;
			cerr << "  My iteration counter: " << iterations << endl;
			cerr << "  Global iteration counter: " << stat_iterations << endl;
		}
		if (active_threads == 0) {
			stat_iterations++;
			cerr << "Iteration " << stat_iterations << " completed." << endl;
			cerr << "  Rows inserted: " << stat_inserts << endl;
			cerr << "  Queries executed: " << stat_queries << endl;
			stat_inserts = 0;
			stat_queries = 0;
		}
#endif
		pthread_mutex_unlock(&global_lock);

		iterations++;
		sleep(sleep_time);
	}
}

RTGConf read_rtg_conf(string filename) {
	ifstream rtgconf(filename.c_str());
	string token;
	RTGConf conf;
	while (rtgconf >> token) {
		if (token == "Interval")
			rtgconf >> conf.interval;
		else if (token == "HighSkewSlop")
			rtgconf >> conf.high_skew_slop;
		else if (token == "LowSkewSlop")
			rtgconf >> conf.low_skew_slop;
		else if (token == "DB_Host")
			rtgconf >> conf.dbhost;
		else if (token == "DB_Database")
			rtgconf >> conf.database;
		else if (token == "DB_User")
			rtgconf >> conf.dbuser;
		else if (token == "DB_Pass")
			rtgconf >> conf.dbpass;
		else if (token == "Threads")
			rtgconf >> conf.threads;
	}
	return conf;
}

string no_semi(string token) {
	size_t semicolon = token.find(';', 0);
	if (semicolon != string::npos)
		token = token.erase(semicolon, 1);
	return token;
}

vector<QueryHost> read_rtg_targets(string filename) {
	vector<QueryHost> hosts;
	ifstream targets(filename.c_str());
	string token;
	QueryHost host;
	QueryRow row;
	int state = 0;
	int nhosts = 0;
	int ntargs = 0;
	while (targets >> token) {
		token = no_semi(token);
		if (state == 0) {
			if (token == "host")
				targets >> host.host;
			else if (token == "{")
				state = 1;
		} else if (state == 1) {
			if (token == "community") {
				targets >> token;
				host.community = no_semi(token);
			} else if (token == "snmpver")
				targets >> host.snmpver;
			else if (token == "target") {
				targets >> token;
				row.oid = no_semi(token);
			} else if (token == "{")
				state = 2;
			else if (token == "}") {
				hosts.push_back(host);
				nhosts++;
				host = QueryHost();
				state = 0;
			}
		} else if (state == 2) {
			if (token == "bits")
				targets >> row.bits;
			else if (token == "table") {
				targets >> token;
				row.table = no_semi(token);
			} else if (token == "id")
				targets >> row.id;
			else if (token == "speed")
			 	targets >> row.speed;
			else if (token == "}") {
				host.rows.push_back(row);
				ntargs++;
				row = QueryRow();
				state = 1;
			}
		}
	}

	targets.close();
	cout << "Read " << ntargs << " targets in " << nhosts << " hosts." << endl;
	return hosts;
}

void* start_thread(void *ptr) {
	thread_loop();
	return NULL;
}

int main (int argc, char * const argv[]) {
	init_snmp("betterpoller");

	config = read_rtg_conf("rtg.conf");
	hosts = read_rtg_targets("targets.cfg");
	cache = vector<ResultCache>(hosts.size());
	pthread_t threads[config.threads];

	cerr << "Polling every " << config.interval << " seconds." << endl;
	cerr << "Starting poll with " << config.threads << " threads." << endl;

	for (unsigned i = 0; i < config.threads; i++) {
		pthread_create(&threads[i], NULL, start_thread, NULL);
	}

	for (unsigned i = 0; i < config.threads; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}
