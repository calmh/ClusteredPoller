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
#ifdef MYSQL
#include <mysql++.h>
#endif

#include "types.h"

#define STATISTICS
#define INTERVAL 30
#define THREADS 16
#define DB "rtg"
#define SERVER "localhost"
#define USER "rtg"
#define PASS "password"

// Global variables.
std::vector<QueryHost> hosts;
std::vector<ResultCache> cache;
#ifdef STATISTICS
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
unsigned active_threads = 0;
unsigned stat_inserts = 0;
unsigned stat_queries = 0;
unsigned stat_iterations = 0;
#endif

std::map<std::string, ResultSet> query(QueryHost qh)
{
	std::map<std::string, ResultSet> rs;

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

	std::list<QueryRow>::iterator it;
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
			counter_t value = 0;
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
					value = (((counter_t)(*vars->val.counter64).high) << 32) + (*vars->val.counter64).low;
					got_value = 1;
					break;

				default:
					// Notify unknown type
					std::cerr << "SNMP get for " << qh.host << " OID " << row.oid
						<< " returned unknown variable type " << (unsigned) vars->type << std::endl;
			}

			// If we got a value, put it in the result set.
			if (got_value) {
				ResultRow r(row.id, value, 0, row.bits, response_time);
				rs[row.table].rows.push_back(r);
			}
		} else {
			std::cerr << "SNMP get for " << qh.host << " OID " << row.oid << " failed." << std::endl;
			if (status == STAT_SUCCESS)
				std::cerr << "  Error in packet: " << snmp_errstring(response->errstat) << std::endl;
			else
				snmp_sess_perror("  Communication error: ", ss);
		}


		if (response)
			snmp_free_pdu(response);
	}

	snmp_sess_close(sessp);

	return rs;
}

void process_host(QueryHost &host, ResultCache &cache) {
#ifdef MYSQL
	mysqlpp::Connection conn(false);
	if (!conn.connect(DB, SERVER, USER, PASS)) {}
#endif

	// Query all values specified in the QueryHost and get back a list of ResultSets.
	// Each ResultSet represents one table in the database.
	std::map<std::string, ResultSet> results = query(host);

	// Iterate over all the ResultSets we got back.
	std::map<std::string, ResultSet>::iterator it;
	for (it = results.begin(); it != results.end(); it++) {
		ResultSet r = it->second;
		if (r.rows.size() > 0) {
			std::stringstream insert_query;
			std::stringstream query_values;
			insert_query << "INSERT INTO " << r.table << " (id, dtime, counter, rate) VALUES ";
			int inserted_rows = 0;

			// Iterate over all the ResultRows in this ResultSet
			std::list<ResultRow>::iterator ri;
			for (ri = r.rows.begin(); ri != r.rows.end(); ri++) {
				ResultRow row = *ri;
				std::pair<std::string, int> key = std::pair<std::string, int>(r.table, row.id);

				// If we have a previous measurement, calculate counter_diff and rate
				// and create an insert statement
				time_t prev_time = cache.times[key];
				if (prev_time != 0) {
					time_t time_diff = row.dtime - prev_time;
					counter_t prev_counter = cache.counters[key];
					counter_t counter_diff = row.counter - prev_counter;
					counter_t rate = 0;
					if (row.bits == 0)
						rate = row.counter;
					else
						rate = counter_diff / time_diff;
					if (inserted_rows > 0)
						query_values << ", ";
					query_values << "(" << row.id << ", now(), " << counter_diff << ", " << rate << ")";
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
#ifdef MYSQL
				mysqlpp::Query q = conn.query(insert_query.str());
#else
				std::cout << insert_query.str() << std::endl;
#endif
			}
		}
	}
}

std::vector<QueryHost> load_hosts() {
	std::vector<QueryHost> hosts;
	std::ifstream targets;
	targets.open("targets.easy");
	std::string token;
	QueryHost host;
	QueryRow row;
	int nhosts = 0;
	int ntargs = 0;
	while (targets.good()) {
		targets >> token;
		if (token == "host") {
			// host 172.16.18.9 dkvavalad07 2
			if (host.host != "none")
				hosts.push_back(host);

			std::string ip;
			std::string community;
			int snmpver;
			targets >> ip >> community >> snmpver;
			host = QueryHost(ip, community, snmpver);
			nhosts++;
		}
		else if (token == "target") {
			// target 1.3.6.1.2.1.10.94.1.1.2.1.5.1001005 0 adslAtucCurrSnrAtn_560 7029 30000000
			std::string oid;
			unsigned bits;
			std::string table;
			unsigned id;
			counter_t speed;
			targets >> oid >> bits >> table >> id >> speed;
			row = QueryRow(oid, table, id, bits);
			host.rows.push_back(row);
			ntargs++;
		}
	}
	if (host.host != "none")
		hosts.push_back(host);
	// std::cout << "good: " << targets.good() << " eof: " << targets.eof() << " fail: " << targets.fail() << " bad: " << targets.bad() << std::endl;
	std::cerr << "Loaded " << ntargs << " targets in " << nhosts << " hosts." << std::endl;
	targets.close();
	return hosts;
}

void* thread_loop(void *ptr) {
	unsigned offset = *((unsigned*) ptr);
	unsigned stride = THREADS;
	std::cerr << "Starting thread with offset " << offset << " and stride " << stride << "." << std::endl;
	unsigned iterations = 0;
	while (1) {
#ifdef STATISTICS
		// Mark ourself active
		pthread_mutex_lock(&global_lock);
		active_threads++;
		pthread_mutex_unlock(&global_lock);
#endif

		time_t start = time(NULL);
		for (unsigned i = offset; i < hosts.size(); i += stride) {
			QueryHost host = hosts[i];
			process_host(host, cache[i]);
		}
		time_t end = time(NULL);
		time_t sleep_time = INTERVAL - (end - start);
		std::cerr << "Thread " << offset << " sleeping " << sleep_time << " seconds." << std::endl;

#ifdef STATISTICS
		// Mark ourself sleeping
		pthread_mutex_lock(&global_lock);
		active_threads--;
		if (iterations < stat_iterations) {
			std::cerr << "Thread " << offset << " is behind schedule!" << std::endl;
			std::cerr << "  My iteration counter: " << iterations << std::endl;
			std::cerr << "  Global iteration counter: " << stat_iterations << std::endl;
		}
		if (active_threads == 0) {
			stat_iterations++;
			std::cerr << "Iteration " << stat_iterations << " completed." << std::endl;
			std::cerr << "  Rows inserted: " << stat_inserts << std::endl;
			std::cerr << "  Queries executed: " << stat_queries << std::endl;
			stat_inserts = 0;
			stat_queries = 0;
		}
		pthread_mutex_unlock(&global_lock);
#endif

		iterations++;
		sleep(sleep_time);
	}
	return NULL;
}

int main (int argc, char * const argv[]) {
	init_snmp("betterpoller");

	unsigned counter_size = sizeof(counter_t)*8;
	if (counter_size != 64) {
		std::cerr << "Counter_t is " << counter_size << " bits." << std::endl;
		std::cerr << "Unfortunately, we need a long long to be 64 bits, or nothing works." << std::endl;
		exit(-1);
	}

	hosts = load_hosts();
	cache = std::vector<ResultCache>(hosts.size());
	pthread_t threads[THREADS];

	for (unsigned i = 0; i < THREADS; i++) {
		pthread_create(&threads[i], NULL, thread_loop, (void*) &i);
		usleep(10);
	}

	for (unsigned i = 0; i < THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}
