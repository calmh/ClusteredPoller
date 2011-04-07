#ifndef _SNMP_H
#define _SNMP_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

struct clsnmp_session {
        struct snmp_session session;
        void *sessp;
};

void clsnmp_global_init();
struct clsnmp_session *clsnmp_session_create(const char *host, const char *community, int snmpver);
void clsnmp_session_free(struct clsnmp_session *session);
int clsnmp_get(struct clsnmp_session *session, const char *oid_str, unsigned long long *counter, time_t *response_time);

#endif
