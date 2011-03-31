#ifndef _QUERYABLEHOST_H
#define _QUERYABLEHOST_H

#include <map>
#include <string>
#include <vector>

#include "snmp.h"
#include "rtgtargets.h"

class ResultCache;
class ResultSet;

class QueryableHost
{
private:
        queryhost* host;
        ResultCache& cache;
        void initialize_result_set(std::map<std::string,ResultSet>& rs, queryrow* row);
        bool query_snmp(clsnmp_session* snmp_session, queryrow* row, std::map<std::string,ResultSet>& rs);
        std::string build_insert_query(ResultSet& r);

public:
        QueryableHost(queryhost* host, ResultCache& cache);
        std::vector<std::string> get_inserts();
        std::pair<unsigned long long, unsigned long long> calculate_rate(time_t prev_time, unsigned long long prev_counter, time_t cur_time, unsigned long long cur_counter, int bits);
        std::map<std::string, ResultSet> get_all_resultsets();
};

#endif
