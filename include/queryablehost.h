#ifndef _QUERYABLEHOST_H
#define _QUERYABLEHOST_H

#include <map>
#include <string>
#include <vector>

class QueryHost;
class ResultCache;
class ResultSet;
class QueryRow;
class SNMP;

class QueryableHost {
private:
	QueryHost& host;
	ResultCache& cache;
    void initialize_result_set(std::map<std::string,ResultSet>& rs, QueryRow& row);
    bool query_snmp(SNMP& snmp_session, QueryRow& row, std::map<std::string,ResultSet>& rs);

public:
	QueryableHost(QueryHost& host, ResultCache& cache);
	std::vector<std::string> process();
	std::pair<uint64_t, uint64_t> calculate_rate(time_t prev_time, uint64_t prev_counter, time_t cur_time, uint64_t cur_counter, int bits);
	std::map<std::string, ResultSet> query_all_targets();
};

#endif
