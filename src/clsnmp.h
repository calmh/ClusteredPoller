//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#ifndef CLSNMP_H
#define CLSNMP_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/// @file clsnmp.h
/// Clpoll SNMP wrapper.

/// An object representing a session to a certain host.
/// @see clsnmp_global_init
/// @see clsnmp_session_create
/// @see clsnmp_session_free
/// @see clsnmp_get
struct clsnmp_session {
        struct snmp_session session;
        void *sessp;
};

/// Global initialization, needs to be called exactly once during program execution,
/// prior to any other clsnmp usage.
void clsnmp_global_init();

/// Create a new clsnmp_session towards the specified host with the specified community and version.
/// @param host The host name or IP.
/// @param community The community name.
/// @param snmpver The SNMP version (1 or 2).
/// @return A new clsnmp object.
struct clsnmp_session *clsnmp_session_create(const char *host, const char *community, int snmpver);

/// Free a clsnmp_session object.
/// @param session The clsnmp_session object to free.
void clsnmp_session_free(struct clsnmp_session *session);

/// Get a value with the specified OID on the the specified session.
/// Stores the result in *counter and the time when the result was received in *response_time.
/// @param session The clsnmp_session to use.
/// @param oid_str The OID to read.
/// @param [out] counter The read counter value.
/// @param [out] response_time The timestamp when the value was read.
/// @return Zero if successfull.
int clsnmp_get(struct clsnmp_session *session, const char *oid_str, unsigned long long *counter, time_t *response_time);

#endif /* CLSNMP_H */
