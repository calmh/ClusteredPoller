#include "snmp.h"

unsigned int speed = 1000000 / 8;

void mock_set_speed(unsigned int newspeed)
{
        speed = newspeed;
}

void clsnmp_global_init()
{
}

clsnmp_session* clsnmp_session_create(const char* host, const char* community, int snmpver)
{
        clsnmp_session* session = (clsnmp_session*) malloc(sizeof(clsnmp_session));
        return session;
}

void clsnmp_session_free(clsnmp_session* session)
{
        free(session);
}

int clsnmp_get(clsnmp_session* session, const char* oid_str, unsigned long long* counter, time_t* response_time)
{
        time(response_time);
        *counter = *response_time * speed;
        return 1;
}

