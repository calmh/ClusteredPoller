#ifndef _SNMP_H
#define _SNMP_H

#include <string>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <pthread.h>
#include <stdexcept>

class SNMPCommunicationException : public std::runtime_error
{
public:
        SNMPCommunicationException(const std::string& what);
};

class SNMP
{
private:
        static bool global_init_done;
        static pthread_mutex_t snmp_lock;
        struct snmp_session session;
        void* sessp;

public:
        SNMP(std::string host, std::string community, int snmpver);
        ~SNMP();
        bool get_counter(std::string oid_str, uint64_t* counter, time_t* response_time);
};

#endif

