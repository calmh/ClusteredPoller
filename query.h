#ifndef _QUERY_H
#define _QUERY_H

#include "types.h"

// See query.cpp for comments.

std::pair<uint64_t, uint64_t> calculate_rate(time_t prev_time, uint64_t prev_counter, time_t cur_time, uint64_t cur_counter, int bits);
void* snmp_init(std::string host, std::string community);
bool snmp_get(void* sessp, std::string oid, uint64_t* counter, time_t* response_time);
std::map<std::string, ResultSet> query(QueryHost qh);
std::vector<std::string> process_host(QueryHost &host, ResultCache &cache);
void thread_loop();
void* poller_thread(void *ptr);
void* monitor_thread(void *ptr);
RTGConf read_rtg_conf(std::string filename);
std::vector<QueryHost> read_rtg_targets(std::string filename, RTGConf &conf);

#endif
