#include <string>
#include <iostream>

#include "types.h"
#include "snmp.h"

using namespace std;

SNMPCommunicationException::SNMPCommunicationException(const string& what)
        : runtime_error(what)
{
}

bool SNMP::global_init_done = false;
pthread_mutex_t SNMP::snmp_lock = PTHREAD_MUTEX_INITIALIZER;

SNMP::SNMP(string host, string community, int snmpver)
{
        // Initialization of SNMP isn't thread safe.
        pthread_mutex_lock(&snmp_lock);
        if (!global_init_done) {
                init_snmp("clpoll");
                global_init_done = true;
        }

        snmp_sess_init(&session);
        session.peername = (char*) host.c_str();
        session.community = (u_char*) community.c_str();
        session.community_len = community.length();
        if (snmpver == 2)
                session.version = SNMP_VERSION_2c;
        else if (snmpver == 1)
                session.version = SNMP_VERSION_1;
        else
                throw SNMPCommunicationException(string("Unsupported SNMP version for host ") + host);
        sessp = snmp_sess_open(&session);

        if (!sessp) {
                throw SNMPCommunicationException(string("Couldn't create an SNMP session for host ") + host);
        }

        snmp_sess_session(sessp);
        pthread_mutex_unlock(&snmp_lock);
}

SNMP::~SNMP()
{
        snmp_sess_close(sessp);
}

// Do an SNMP GET for a value, and return true if successfull.
// Updates the counter and response_time parameters.
bool SNMP::get_counter(string oid_str, unsigned long long* counter, time_t* response_time)
{
        struct snmp_pdu* pdu;
        struct snmp_pdu* response;
        oid anOID[MAX_OID_LEN];
        size_t anOID_len = MAX_OID_LEN;
        int status;

        pdu = snmp_pdu_create(SNMP_MSG_GET);
        read_objid(oid_str.c_str(), anOID, &anOID_len);
        snmp_add_null_var(pdu, anOID, anOID_len);

        status = snmp_sess_synch_response(sessp, pdu, &response);
        time(response_time);

        bool success = false;
        if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
                struct variable_list* vars = response->variables;
                switch (vars->type) {
                case SNMP_NOSUCHOBJECT:
                case SNMP_NOSUCHINSTANCE:
                        // Do nothing
                        break;

                case ASN_INTEGER:
                case ASN_COUNTER:
                case ASN_GAUGE:
                case ASN_OPAQUE:
                        // Regular integer
                        *counter = *vars->val.integer;
                        success = true;
                        break;

                case ASN_COUNTER64:
                        // Get high and low 32 bits and shift them together
                        *counter = (((unsigned long long)(*vars->val.counter64).high) << 32) + (*vars->val.counter64).low;
                        success = true;
                        break;
                }
        }

        if (response)
                snmp_free_pdu(response);

        return success;
}
