#define MAXERRORSPERHOST 3

#include "types.h"
#include "query.h"
#include "globals.h"
#include "util.h"
#include "snmp.h"

using namespace std;

// Query all targets for the specified host and return a collection of result sets.
map<string, ResultSet> query(QueryHost qh)
{
	// Allocate our resultsets.
	// We map from table name to result set.
	map<string, ResultSet> rs;

	// Start a new SNMP session.
	void *sessp = snmp_init_session(qh.host, qh.community);
	if (!sessp)
		return rs;

	int errors = 0;
	// Iterate over all targets in the host.
	vector<QueryRow>::iterator it;
	for (it = qh.rows.begin(); it != qh.rows.end(); it++) {
		QueryRow row = *it;
		// Check if there is an existing result set for this table in the map,
		// or create a new one.
		if (rs.find(row.table) == rs.end()) {
			ResultSet r(row.table);
			rs[row.table] = r;
		}

		uint64_t value;
		time_t response_time;
		if (snmp_get(sessp, row.oid, &value, &response_time)) {
			// We got a result from SNMP polling, so insert it into the result set.
			ResultRow r(row.id, value, 0, row.bits, response_time, row.speed);
			rs[row.table].rows.push_back(r);
		} else {
			if (verbosity >= 1) {
				// Inform about the failure.
				pthread_mutex_lock(&cerr_lock);
				cerr << "SNMP get for " << qh.host << " OID " << row.oid << " failed." << endl;
				pthread_mutex_unlock(&cerr_lock);
			}
			errors++;
			if (errors >= MAXERRORSPERHOST) {
				// We have done enough attempts with this host, so lets not waste any more time on it.
				if (verbosity >= 1) {
					pthread_mutex_lock(&cerr_lock);
					cerr << "Too many errors for host " << qh.host << ", aborting." << endl;
					pthread_mutex_unlock(&cerr_lock);
				}
				break;
			}
		}

	}

	// Close the SNMP session we started before.
	snmp_close_session(sessp);

	return rs;
}

// Calculate the traffic rate between to points, for a given counter size (bits).
// bits == 0 means it's a 32 bit gauge (RTG legacy).
// bits == 32 or 64 means it's that size of counter.
pair<uint64_t, uint64_t> calculate_rate(time_t prev_time, uint64_t prev_counter, time_t cur_time, uint64_t cur_counter, int bits)
{
	time_t time_diff = cur_time - prev_time;
	uint64_t counter_diff = cur_counter - prev_counter;
	if (prev_counter > cur_counter) {
		// We seem to have a wrap.
		// Wrap it back to find the correct rate.
		if (bits == 64)
			counter_diff += 18446744073709551615ull + 1; // 2^64-1 + 1
		else
			counter_diff += 4294967296ull; // 2^32
	}

	if (bits == 0)
		// It's a gauge so just return the value as both counter diff and rate.
		return pair<uint64_t, uint64_t>(cur_counter, cur_counter);
	else
		// Return the calculated rate.
		return pair<uint64_t, uint64_t>(counter_diff, counter_diff / time_diff);
}

// Query all targets for a host, process rates compared with cache, and return vector of database queries
// that are ready to be executed.
vector<string> process_host(QueryHost &host, ResultCache &cache)
{
	// Store all database queries here for later processing.
	vector<string> queries;

	if (verbosity >= 3) {
		pthread_mutex_lock(&cerr_lock);
		cerr << "process_host(" << host.host << ") running query()" << endl;
		pthread_mutex_unlock(&cerr_lock);
	}

	// Query all values specified in the QueryHost and get back a list of ResultSets.
	// Each ResultSet represents one table in the database.
	map<string, ResultSet> results = query(host);

	if (verbosity >= 3) {
		pthread_mutex_lock(&cerr_lock);
		cerr << "process_host(" << host.host << ") got " << results.size() << " tables back grom query()" << endl;
		pthread_mutex_unlock(&cerr_lock);
	}
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
				// Create a hash key from table name and row id.
				pair<string, int> key = pair<string, int>(r.table, row.id);
				if (cache.times.find(key) != cache.times.end()) {
					// We have a cache entry, so we can calculate rate since last measurement.
					time_t prev_time = cache.times[key];
					uint64_t prev_counter = cache.counters[key];

					// Get the rate, corrected for wraps etc.
					pair<uint64_t, uint64_t> rate = calculate_rate(prev_time, prev_counter, row.dtime, row.counter, row.bits);

					// Verify that the resulting value is reasonable, i.e. lower than interface speed.
					if (rate.second <= row.speed) {
						// Check if we should insert it, based on whether or not we want db zeroes and whether it's a gauge or not.
						if (allow_db_zero || (row.bits != 0 && rate.second > 0) || (row.bits == 0 && row.counter != prev_counter)) {
							if (inserted_rows > 0)
								insert_query << ", ";
							// Build on the insert query. We set dtime to the time returned from snmp_get.
							insert_query << "(" << row.id << ", FROM_UNIXTIME(" << row.dtime << "), " << rate.first << ", " << rate.second << ")";
							inserted_rows++;
						}
					}
				}

				// Update the cache for next iteration.
				cache.counters[key] = row.counter;
				cache.times[key] = row.dtime;
			}

			// Update statistics.
			if (inserted_rows > 0) {
				pthread_mutex_lock(&global_lock);
				stat_inserts += inserted_rows;
				stat_queries ++;
				pthread_mutex_unlock(&global_lock);
				queries.push_back(insert_query.str());
			}
		}
	}

	if (verbosity >= 3) {
		pthread_mutex_lock(&cerr_lock);
		cerr << "process_host(" << host.host << ") returning " << queries.size() << " queries" << endl;
		pthread_mutex_unlock(&cerr_lock);
	}
	// Return all the insert queries for processing.
	return queries;
}

// Continuosly loop over all the hosts and do all processing for them.
// Each thread has an offset (= thread id) and stride (= number of threads).
// The list of hosts is processed starting at "offset" and then incrementing
// with "stride" each iteration. This way we avoid having a separate host list
// for each thread, and the threads keep off each others toes.
void* poller_thread(void *ptr)
{
	// Assign our offset and stride values.
	unsigned stride = config.threads;
	pthread_mutex_lock(&global_lock);
	unsigned offset = thread_id++;
	pthread_mutex_unlock(&global_lock);

	// Start looping.
	unsigned iterations = 0;
	time_t start = 0, end = 0;
	while (1) {

		// Mark ourself sleeping
		if (iterations > 0 && verbosity >= 2) {
			pthread_mutex_lock(&cerr_lock);
			cerr << "Thread " << offset << " sleeping after " << end - start << " s processing time." << endl;
			pthread_mutex_unlock(&cerr_lock);
		}

		// Wait for green light.
		pthread_mutex_lock(&global_lock);
		if (iterations > 0)
			active_threads--;
		pthread_cond_wait(&global_cond, &global_lock);
		pthread_mutex_unlock(&global_lock);

		if (verbosity >= 2) {
			pthread_mutex_lock(&cerr_lock);
			cerr << "Thread " << offset << " starting." << endl;
			pthread_mutex_unlock(&cerr_lock);
		}
		// Mark ourself active.
		pthread_mutex_lock(&global_lock);
		active_threads++;
		pthread_mutex_unlock(&global_lock);

		// Note our start time, so we know how long an iteration takes.
		start = time(NULL);
		// Loop over our share of the hosts.
		for (unsigned i = offset; i < hosts.size(); i += stride) {
			QueryHost host = hosts[i];
			if (verbosity >= 2) {
				pthread_mutex_lock(&cerr_lock);
				cerr << "Thread " << offset << " picked host #" << i << ": " << host.host << "." << endl;
				pthread_mutex_unlock(&cerr_lock);
			}
			// Process the host and get back a list of SQL updates to execute.
			vector<string> host_queries = process_host(host, cache[i]);

			if (host_queries.size() > 0) {
				if (verbosity >= 2) {
					pthread_mutex_lock(&cerr_lock);
					cerr << "Thread " << offset << " queueing " << host_queries.size() << " queries." << endl;
					pthread_mutex_unlock(&cerr_lock);
				}

				vector<string>::iterator it;
				pthread_mutex_lock(&db_list_lock);
				for (it = host_queries.begin(); it != host_queries.end(); it++)	 {
					queries.push_back(*it);
				}
				unsigned qd = queries.size();
				query_queue_depth = query_queue_depth > qd ? query_queue_depth : qd;
				pthread_mutex_unlock(&db_list_lock);

			}
		}

		// Note how long it took.
		end = time(NULL);

		// Prepare for next iteration.
		iterations++;
	}
	return NULL;
}

void* monitor_thread(void *ptr)
{
	time_t interval = 0; //(time(NULL) / config.interval + 1) * config.interval;
	int in_iteration = 0;
	while (1) {
		sleep(1);
		if (active_threads == 0 && in_iteration) {
			stat_iterations++;
			if (verbosity >= 1) {
				pthread_mutex_lock(&db_list_lock);
				unsigned qd = queries.size();
				pthread_mutex_unlock(&db_list_lock);
				pthread_mutex_lock(&cerr_lock);
				cerr << "Monitor sees everyone is complete. Elapsed time for this iteration #" << stat_iterations << " was " << time(NULL) - in_iteration << "s." << endl;
				cerr << "Time until next iteration is " << interval - time(NULL) << "s." << endl;
				cerr << "  Rows inserted: " << stat_inserts << endl;
				cerr << "  Queries queued: " << stat_queries << endl;
				cerr << "  Max queue depth: " << query_queue_depth << endl;
				cerr << "  Remaining queue: " << qd << endl;
				pthread_mutex_unlock(&cerr_lock);
			}
			stat_inserts = 0;
			stat_queries = 0;
			in_iteration = 0;
			query_queue_depth = 0;
		}

		pthread_mutex_lock(&global_lock);
		if (active_threads == 0 && time(NULL) > interval) {
			interval = (time(NULL) / config.interval + 1) * config.interval;
			in_iteration = time(NULL);
			if (verbosity >= 1) {
				pthread_mutex_lock(&cerr_lock);
				cerr << "Monitor signals wakeup." << endl;
				pthread_mutex_unlock(&cerr_lock);
			}
			pthread_cond_broadcast(&global_cond);
		}
		pthread_mutex_unlock(&global_lock);
	}
	return NULL;
}

// Exctract a query from the queue, or an empty string if it's empty.
string dequeue_query()
{
	pthread_mutex_lock(&db_list_lock);
	unsigned qs = queries.size();
	if (qs == 0)
		return "";
	string q = queries.front();
	queries.pop_front();
	pthread_mutex_unlock(&db_list_lock);
	return q;
}

void* database_thread(void *ptr)
{
	pthread_mutex_lock(&global_lock);
	unsigned my_id = thread_id++;
	pthread_mutex_unlock(&global_lock);

#ifdef USE_MYSQL
	mysqlpp::Connection conn(false);
	int useless_iterations = 0;
#endif
	while (1) {
		pthread_mutex_lock(&db_list_lock);
		unsigned qs = queries.size();
		pthread_mutex_unlock(&db_list_lock);
		if (qs > 0) {
#ifdef USE_MYSQL
			if (!conn.connected()) {
				if (verbosity >= 2) {
					pthread_mutex_lock(&cerr_lock);
					cerr << "DB thread " << my_id << " connecting to MySQL" << endl;
					pthread_mutex_unlock(&cerr_lock);
				}
				conn.connect(config.database.c_str(), config.dbhost.c_str(), config.dbuser.c_str(), config.dbpass.c_str());
				if (!conn.connected()) {
					pthread_mutex_lock(&cerr_lock);
					cerr << "DB thread " << my_id << " !! Connection failed." << endl;
					pthread_mutex_unlock(&cerr_lock);
				}
			} else {
				useless_iterations = 0;
				mysqlpp::Query q = conn.query(dequeue_query());
				q.exec();
			}
#else
			cerr << dequeue_query() << endl;
#endif
		} else {
#ifdef USE_MYSQL
			useless_iterations++;
			if (useless_iterations > 10 && conn.connected()) {
				if (verbosity >= 2) {
					pthread_mutex_lock(&cerr_lock);
					cerr << "DB thread " << my_id << " disconnecting from MySQL" << endl;
					pthread_mutex_unlock(&cerr_lock);
				}
				conn.disconnect();
			}
#endif
			sleep(1);
		}
	}
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
		else if (token == "highskewslop") // Not sure what these are, possibly something for rtgplot to determine when data is too old to plot etc?
			rtgconf >> conf.high_skew_slop;
		else if (token == "lowskewslop") // Not sure what these are, possibly something for rtgplot to determine when data is too old to plot etc?
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

// Simple state machine based config reader.
vector<QueryHost> read_rtg_targets(string filename, RTGConf &conf)
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
			if (token == "host") {
				host = QueryHost();
				targets >> host.host;
			}
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
				row = QueryRow();
				row.oid = no_semi(token);
			}
			else if (token == "{") {
				state = 2;
			}
			else if (token == "}") {
				hosts.push_back(host);
				nhosts++;
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
				uint64_t max_counter_diff;
				targets >> max_counter_diff;
				if (row.bits == 0)
					row.speed = max_counter_diff;
				else
					row.speed = max_counter_diff / conf.interval;
			}
			else if (token == "}") {
				host.rows.push_back(row);
				ntargs++;
				state = 1;
			}
		}
	}

	targets.close();
	if (verbosity >= 1)
		cerr << "Read " << ntargs << " targets in " << nhosts << " hosts." << endl;
	return hosts;
}
