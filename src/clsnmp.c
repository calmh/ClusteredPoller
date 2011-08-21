/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#include "clsnmp.h"

#include "xmalloc.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <pthread.h>

struct clsnmp_session {
        void *sessp;            /* SNMP library session pointer. */
};

void clsnmp_global_init(void)
{
        struct snmp_session libsnmp_session;
        init_snmp("clpoll");
        /* This loads and initializes global structures before we start threading. */
        snmp_sess_init(&libsnmp_session);
}

struct clsnmp_session *clsnmp_session_create(const char *host, const char *community, int snmpver)
{
        struct clsnmp_session *session;
        struct snmp_session libsnmp_session;

        if (snmpver != 1 && snmpver != 2)
                return NULL;

        session = (struct clsnmp_session *) xmalloc(sizeof(struct clsnmp_session));
        snmp_sess_init(&libsnmp_session);

        libsnmp_session.peername = (char *) host;
        libsnmp_session.community = (u_char *) community;
        libsnmp_session.community_len = strlen(community);
        if (snmpver == 2)
                libsnmp_session.version = SNMP_VERSION_2c;
        else
                libsnmp_session.version = SNMP_VERSION_1;

        session->sessp = snmp_sess_open(&libsnmp_session);

        if (!session->sessp) {
                free(session);
                return NULL;
        }

        snmp_sess_session(session->sessp);

        return session;
}

void clsnmp_session_free(struct clsnmp_session *session)
{
        snmp_sess_close(session->sessp);
        free(session);
}

int clsnmp_get(struct clsnmp_session *session, const char *oid_str, unsigned long long *counter, time_t *response_time)
{
        struct snmp_pdu *pdu;
        struct snmp_pdu *response;
        oid anOID[MAX_OID_LEN];
        size_t anOID_len = MAX_OID_LEN;
        int status;
        int success = 0;

        pdu = snmp_pdu_create(SNMP_MSG_GET);
        read_objid(oid_str, anOID, &anOID_len);
        snmp_add_null_var(pdu, anOID, anOID_len);

        status = snmp_sess_synch_response(session->sessp, pdu, &response);
        time(response_time);

        if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
                struct variable_list *vars = response->variables;
                switch (vars->type) {
                case ASN_INTEGER:
                case ASN_COUNTER:
                case ASN_GAUGE:
                case ASN_OPAQUE:
                        /* Regular integer */
                        *counter = *vars->val.integer;
                        success = 1;
                        break;

                case ASN_COUNTER64:
                        /* Get high and low 32 bits and shift them together */
                        *counter = (((unsigned long long) (*vars->val.counter64).high) << 32) + (*vars->val.counter64).low;
                        success = 1;
                        break;

                default:
                        /* Ignore anything we don't recognize. */
                        break;
                }
        }

        if (response)
                snmp_free_pdu(response);

        return success;
}
