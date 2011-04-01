#include "snmp.h"

static pthread_mutex_t clsnmp_lock = PTHREAD_MUTEX_INITIALIZER;

void clsnmp_global_init()
{
        /* Shouldn't need to lock here since this should only happen once, but hey... */
        pthread_mutex_lock(&clsnmp_lock);
        init_snmp("clpoll");
        pthread_mutex_unlock(&clsnmp_lock);
}

clsnmp_session *clsnmp_session_create(const char *host, const char *community, int snmpver)
{
        if (snmpver != 1 && snmpver != 2)
                return NULL;

        pthread_mutex_lock(&clsnmp_lock);
        clsnmp_session *session = (clsnmp_session *) malloc(sizeof(clsnmp_session));
        snmp_sess_init(&session->session);
        session->session.peername = (char *) host;
        session->session.community = (u_char *) community;
        session->session.community_len = strlen(community);
        if (snmpver == 2)
                session->session.version = SNMP_VERSION_2c;
        else
                session->session.version = SNMP_VERSION_1;

        session->sessp = snmp_sess_open(&session->session);

        if (!session->sessp) {
                pthread_mutex_unlock(&clsnmp_lock);
                free(session);
                return NULL;
        }

        snmp_sess_session(session->sessp);
        pthread_mutex_unlock(&clsnmp_lock);

        return session;
}

void clsnmp_session_free(clsnmp_session *session)
{
        pthread_mutex_lock(&clsnmp_lock);
        snmp_sess_close(session->sessp);
        pthread_mutex_unlock(&clsnmp_lock);
        free(session);
}

int clsnmp_get(clsnmp_session *session, const char *oid_str, unsigned long long *counter, time_t *response_time)
{
        struct snmp_pdu *pdu;
        struct snmp_pdu *response;
        oid anOID[MAX_OID_LEN];
        size_t anOID_len = MAX_OID_LEN;
        int status;

        pdu = snmp_pdu_create(SNMP_MSG_GET);
        read_objid(oid_str, anOID, &anOID_len);
        snmp_add_null_var(pdu, anOID, anOID_len);

        status = snmp_sess_synch_response(session->sessp, pdu, &response);
        time(response_time);

        int success = 0;
        if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
                struct variable_list *vars = response->variables;
                switch (vars->type) {
                case SNMP_NOSUCHOBJECT:
                case SNMP_NOSUCHINSTANCE:
                        /* Do nothing */
                        break;

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
                        *counter = (((unsigned long long)(*vars->val.counter64).high) << 32) + (*vars->val.counter64).low;
                        success = 1;
                        break;
                }
        }

        if (response)
                snmp_free_pdu(response);

        return success;
}
