#include <iostream>
#include <sstream>

#include "types.h"
#include "rtgtargets.h"
#include "queryablehost.h"
#include "globals.h"
#include "util.h"
#include "snmp.h"

using namespace std;

#define MAXERRORSPERHOST 3

QueryableHost::QueryableHost(QueryHost& host, ResultCache& cache) :
        host(host), cache(cache)
{
}

void QueryableHost::initialize_result_set(map<string, ResultSet> & rs, QueryRow& row)
{
        // Check if there is an existing result set for this table in the map,
        // or create a new one.
        if (rs.find(row.table) == rs.end()) {
                ResultSet r(row.table);
                rs[row.table] = r;
        }
}

bool QueryableHost::query_snmp(SNMP& snmp_session, QueryRow& row, map<string, ResultSet>& rs)
{
        unsigned long long value;
        time_t response_time;
        bool success = snmp_session.get_counter(row.oid, &value, &response_time);
        if (success) {
                // We got a result from SNMP polling, so insert it into the result set.
                ResultRow r(row.id, value, 0, row.bits, response_time, row.speed);
                rs[row.table].rows.push_back(r);
        }
        return success;
}

// Query all targets for the specified host and return a collection of result sets.
map<string, ResultSet> QueryableHost::get_all_resultsets()
{
        // Allocate our resultsets.
        // We map from table name to result set.
        map<string, ResultSet> rs;

        try {
                // Start a new SNMP session.
                SNMP snmp_session(host.host, host.community, host.snmpver);

                int errors = 0;
                // Iterate over all targets in the host.
                vector<QueryRow>::iterator it;
                for (it = host.rows.begin(); it != host.rows.end() && errors < MAXERRORSPERHOST; it++) {
                        QueryRow row = *it;
                        initialize_result_set(rs, row);
                        bool success = query_snmp(snmp_session, row, rs);
                        if (!success) {
                                log(1, "SNMP get for %s OID %s failed.", host.host.c_str(), row.oid.c_str());
                                errors++;
                        }
                }
                if (errors >= MAXERRORSPERHOST)
                        log(0, "Too many errors for host %s, aborted.", host.host.c_str());
        } catch (SNMPCommunicationException& e) {
                log(0, "Error in SNMP setup.");
                log(0, "%s", e.what());
        }

        return rs;
}

// Calculate the traffic rate between to points, for a given counter size (bits).
// bits == 0 means it's a 32 bit gauge (RTG legacy).
// bits == 32 or 64 means it's that size of counter.
pair<unsigned long long, unsigned long long> QueryableHost::calculate_rate(time_t prev_time, unsigned long long prev_counter, time_t cur_time, unsigned long long cur_counter, int bits)
{
        time_t time_diff = cur_time - prev_time;
        if (time_diff == 0) {
                log(0, "Fatal error: time_diff == 0 (can't happen!)");
                exit(-1);
        }
        unsigned long long counter_diff = cur_counter - prev_counter;
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
                return pair<unsigned long long, unsigned long long> (cur_counter, cur_counter);
        else
                // Return the calculated rate.
                return pair<unsigned long long, unsigned long long> (counter_diff, counter_diff / time_diff);
}

// Query all targets for a host, process rates compared with cache, and return vector of database queries
// that are ready to be executed.
vector<string> QueryableHost::get_inserts()
{
        // Store all database queries here for later processing.
        vector<string> queries;

        // Query all values specified in the QueryHost and get back a list of ResultSets.
        // Each ResultSet represents one table in the database.
        map<string, ResultSet> results = get_all_resultsets();

        // Iterate over all the ResultSets we got back.
        map<string, ResultSet>::iterator it;
        for (it = results.begin(); it != results.end(); it++) {
                ResultSet r = it->second;
                if (r.rows.size() > 0) {
                        string insert_query = build_insert_query(r);
                        if (insert_query != "") {
                                queries.push_back(insert_query);
                        }
                }
        }

        // Return all the insert queries for processing.
        return queries;
}

string QueryableHost::build_insert_query(ResultSet& r)
{
        ostringstream query_stream;
        query_stream << "INSERT INTO " << r.table << " (id, dtime, counter, rate) VALUES ";

        // Iterate over all the ResultRows in this ResultSet
        vector<ResultRow>::iterator ri;
        int rows = 0;
        for (ri = r.rows.begin(); ri != r.rows.end(); ri++) {
                ResultRow row = *ri;
                // Create a hash key from table name and row id.
                pair<string, int> key = pair<string, int> (r.table, row.id);
                if (cache.times.find(key) != cache.times.end()) {
                        // We have a cache entry, so we can calculate rate since last measurement.
                        time_t prev_time = cache.times[key];
                        unsigned long long prev_counter = cache.counters[key];

                        // Get the rate, corrected for wraps etc.
                        pair<unsigned long long, unsigned long long> rate = calculate_rate(prev_time, prev_counter, row.dtime, row.counter, row.bits);

                        // Verify that the resulting value is reasonable, i.e. lower than interface speed.
                        if (rate.second <= row.speed) {
                                // Check if we should insert it, based on whether or not we want db zeroes and whether it's a gauge or not.
                                if (allow_db_zero || (row.bits != 0 && rate.second > 0) || (row.bits == 0 && row.counter != prev_counter)) {
                                        // Build on the insert query. We set dtime to the time returned from snmp_get.
                                        if (rows > 0)
                                                query_stream << ", ";
                                        query_stream << "(" << row.id << ", FROM_UNIXTIME("
                                                     << row.dtime << "), " << rate.first << ", "
                                                     << rate.second << ")";
                                        rows++;
                                }
                        }
                }

                // Update the cache for next iteration.
                cache.counters[key] = row.counter;
                cache.times[key] = row.dtime;
        }

        if (rows > 0) {
                // Update the statistics
                pthread_mutex_lock(&global_lock);
                stat_inserts += rows;
                stat_queries++;
                pthread_mutex_unlock(&global_lock);

                return query_stream.str();
        } else {
                return "";
        }
}
