#ifndef _SNMP_H
#define _SNMP_H

#include <string>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

// See snmp.cpp for comments.

void global_snmp_init();
void* snmp_init_session(std::string host, std::string community);
void snmp_close_session(void* sessp);
bool snmp_get(void* sessp, std::string oid_str, uint64_t* counter, time_t* response_time);

#endif

