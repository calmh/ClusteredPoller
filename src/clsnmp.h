#ifndef _SNMP_H
#define _SNMP_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

// An object representing a session to a certain host.
struct clsnmp_session {
        struct snmp_session session;
        void *sessp;
};

// Global initialization, needs to be called exactly once during program execution,
// prior to any other clsnmp usage.
void clsnmp_global_init();

// Create a new sesstion towards the specified host with the specified community and version.
struct clsnmp_session *clsnmp_session_create(const char *host, const char *community, int snmpver);

// Free a session object.
void clsnmp_session_free(struct clsnmp_session *session);

// Get a value with the specified OID on the the specified session.
// Stores the result in *counter and the time when the result was received in *response_time.
int clsnmp_get(struct clsnmp_session *session, const char *oid_str, unsigned long long *counter, time_t *response_time);

#endif
