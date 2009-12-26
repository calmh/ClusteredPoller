#ifndef _QUERY_H
#define _QUERY_H

#include "types.h"

void* snmp_init(std::string host, std::string community);
bool snmp_get(void* sessp, std::string oid, uint64_t* counter, time_t* response_time);
std::map<std::string, ResultSet> query(QueryHost qh);
void process_host(QueryHost &host, ResultCache &cache);
void thread_loop();
void* start_thread(void *ptr);
RTGConf read_rtg_conf(std::string filename);
std::vector<QueryHost> read_rtg_targets(std::string filename);

#endif
