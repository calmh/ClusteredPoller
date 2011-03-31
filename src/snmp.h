#ifndef _SNMP_H
#define _SNMP_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <pthread.h>

typedef struct {
        struct snmp_session session;
        void* sessp;
} clsnmp_session;

#ifdef __cplusplus 
extern "C" void clsnmp_global_init();
extern "C" clsnmp_session* clsnmp_session_create(const char* host, const char* community, int snmpver);
extern "C" void clsnmp_session_free(clsnmp_session* session);
extern "C" int clsnmp_get(clsnmp_session* session, const char* oid_str, unsigned long long* counter, time_t* response_time);
#else
void clsnmp_global_init();
clsnmp_session* clsnmp_session_create(const char* host, const char* community, int snmpver);
void clsnmp_session_free(clsnmp_session* session);
int clsnmp_get(clsnmp_session* session, const char* oid_str, unsigned long long* counter, time_t* response_time);
#endif

#endif

