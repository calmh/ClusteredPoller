#define MAXERRORSPERHOST 4

#include "types.h"
#include "query.h"
#include "globals.h"
#include "util.h"
#include "snmp.h"

using namespace std;

map<string, ResultSet> query(QueryHost qh)
{
	map<string, ResultSet> rs;

	void *sessp = snmp_init_session(qh.host, qh.community);
	if (!sessp)
		return rs;

	int errors = 0;
	vector<QueryRow>::iterator it;
	for (it = qh.rows.begin(); it != qh.rows.end(); it++) {
		QueryRow row = *it;
		if (rs.find(row.table) == rs.end()) {
			ResultSet r(row.table);
			rs[row.table] = r;
		}

		uint64_t value;
		time_t response_time;
		if (snmp_get(sessp, row.oid, &value, &response_time)) {
			ResultRow r(row.id, value, 0, row.bits, response_time);
			rs[row.table].rows.push_back(r);
		} else {
			if (verbosity >= 1) {
				pthread_mutex_lock(&global_lock);
				cerr << "SNMP get for " << qh.host << " OID " << row.oid << " failed." << endl;
				pthread_mutex_unlock(&global_lock);
			}
			errors++;
			if (errors > MAXERRORSPERHOST) {
				if (verbosity >= 1) {
					pthread_mutex_lock(&global_lock);
					cerr << "Too many errors for host " << qh.host << ", aborting." << endl;
					pthread_mutex_unlock(&global_lock);
				}
				break;
			}
		}

	}

	snmp_close_session(sessp);

	return rs;
}

pair<uint64_t, uint64_t> calculate_rate(time_t prev_time, uint64_t prev_counter, time_t cur_time, uint64_t cur_counter, int bits)
{
	time_t time_diff = cur_time - prev_time;
	uint64_t counter_diff = cur_counter - prev_counter;
	if (prev_counter > cur_counter) {
		if (bits == 64)
			counter_diff += 18446744073709551615ull + 1; // 2^64-1 + 1
		else
			counter_diff += 4294967296ull; // 2^32
	}

	if (bits == 0)
		return pair<uint64_t, uint64_t>(cur_counter, cur_counter);
	else
		return pair<uint64_t, uint64_t>(counter_diff, counter_diff / time_diff);
}

vector<string> process_host(QueryHost &host, ResultCache &cache)
{
	// Store all database queries here for later processing.
	vector<string> queries;

	// Query all values specified in the QueryHost and get back a list of ResultSets.
	// Each ResultSet represents one table in the database.
	map<string, ResultSet> results = query(host);

	// Iterate over all the ResultSets we got back.
	map<string, ResultSet>::iterator it;
	for (it = results.begin(); it != results.end(); it++) {
		ResultSet r = it->second;
		if (r.rows.size() > 0) {
			stringstream insert_query;
			insert_query << "INSERT INTO " << r.table << " (id, dtime, counter, rate) VALUES ";
			int inserted_rows = 0;

			// Iterate over all the ResultRows in this ResultSet
			vector<ResultRow>::iterator ri;
			for (ri = r.rows.begin(); ri != r.rows.end(); ri++) {
				ResultRow row = *ri;
				pair<string, int> key = pair<string, int>(r.table, row.id);
				if (cache.times.find(key) != cache.times.end()) {
					time_t prev_time = cache.times[key];
					uint64_t prev_counter = cache.counters[key];
					pair<uint64_t, uint64_t> rate = calculate_rate(prev_time, prev_counter, row.dtime, row.counter, row.bits);

					if (allow_db_zero
						|| (row.bits != 0 && rate.second > 0)
					|| (row.bits == 0 && row.counter != prev_counter)) {
						if (inserted_rows > 0)
							insert_query << ", ";
						insert_query << "(" << row.id << ", FROM_UNIXTIME(" << row.dtime << "), " << rate.first << ", " << rate.second << ")";
						inserted_rows++;
					}
				}
				// Update the cache for next iteration
				cache.counters[key] = row.counter;
				cache.times[key] = row.dtime;
			}

			if (inserted_rows > 0) {
				pthread_mutex_lock(&global_lock);
				stat_inserts += inserted_rows;
				stat_queries ++;
				pthread_mutex_unlock(&global_lock);
				queries.push_back(insert_query.str());
			}
		}
	}
	return queries;
}

void thread_loop()
{
	unsigned stride = config.threads;
	pthread_mutex_lock(&global_lock);
	unsigned offset = thread_id++;
	pthread_mutex_unlock(&global_lock);
	unsigned iterations = 0;
	while (1) {
		// Mark ourself active
		pthread_mutex_lock(&global_lock);
		active_threads++;
		pthread_mutex_unlock(&global_lock);

		time_t start = time(NULL);
		for (unsigned i = offset; i < hosts.size(); i += stride) {
			QueryHost host = hosts[i];
			vector<string> queries = process_host(host, cache[i]);

			if (queries.size() > 0) {
#ifdef USE_MYSQL
				if (use_db) {
					mysqlpp::Connection conn(true);
					conn.connect(config.database.c_str(), config.dbhost.c_str(), config.dbuser.c_str(), config.dbpass.c_str());
					vector<string>::iterator it;
					for (it = queries.begin(); it != queries.end(); it++)	 {
						mysqlpp::Query q = conn.query(*it);
						q.exec();
					}
				} else {
#endif
					vector<string>::iterator it;
					for (it = queries.begin(); it != queries.end(); it++)	 {
						cerr << *it << endl;
					}
#ifdef USE_MYSQL
				}
#endif
			}
		}
		sleep(1);
		time_t end = time(NULL);
		time_t sleep_time = config.interval - (end - start);

		pthread_mutex_lock(&global_lock);
		// Mark ourself sleeping
		active_threads--;
		if (verbosity >= 1) {
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
		}
		pthread_mutex_unlock(&global_lock);

		iterations++;
		sleep(sleep_time);
	}
}

void* start_thread(void *ptr)
{
	thread_loop();
	return NULL;
}

// Configuration reading.

RTGConf read_rtg_conf(string filename)
{
	ifstream rtgconf(filename.c_str());
	string token;
	RTGConf conf;
	while (rtgconf >> token) {
		string_tolower(token);
		if (token == "interval")
			rtgconf >> conf.interval;
		else if (token == "highskewslop")
			rtgconf >> conf.high_skew_slop;
		else if (token == "lowskewslop")
			rtgconf >> conf.low_skew_slop;
		else if (token == "db_host")
			rtgconf >> conf.dbhost;
		else if (token == "db_database")
			rtgconf >> conf.database;
		else if (token == "db_user")
			rtgconf >> conf.dbuser;
		else if (token == "db_pass")
			rtgconf >> conf.dbpass;
		else if (token == "threads")
			rtgconf >> conf.threads;
	}
	return conf;
}

vector<QueryHost> read_rtg_targets(string filename)
{
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
		string_tolower(token);
		if (state == 0) {
			if (token == "host")
				targets >> host.host;
			else if (token == "{")
				state = 1;
		}
		else if (state == 1) {
			if (token == "community") {
				targets >> token;
				host.community = no_semi(token);
			}
			else if (token == "snmpver") {
				targets >> host.snmpver;
			}
			else if (token == "target") {
				targets >> token;
				row.oid = no_semi(token);
			}
			else if (token == "{") {
				state = 2;
			}
			else if (token == "}") {
				hosts.push_back(host);
				nhosts++;
				host = QueryHost();
				state = 0;
			}
		}
		else if (state == 2) {
			if (token == "bits")
				targets >> row.bits;
			else if (token == "table") {
				targets >> token;
				row.table = no_semi(token);
			}
			else if (token == "id") {
				targets >> row.id;
			}
			else if (token == "speed") {
				targets >> row.speed;
			}
			else if (token == "}") {
				host.rows.push_back(row);
				ntargs++;
				row = QueryRow();
				state = 1;
			}
		}
	}

	targets.close();
	if (verbosity >= 1)
		cerr << "Read " << ntargs << " targets in " << nhosts << " hosts." << endl;
	return hosts;
}
