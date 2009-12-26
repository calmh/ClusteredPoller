#include "snmp.h"

#include <string>
#include <iostream>

using namespace std;

void global_snmp_init()
{
}

void* snmp_init_session(string host, string community)
{
	return (void*)1000;
}

void snmp_close_session(void* sessp)
{
}

bool snmp_get(void* sessp, string oid_str, uint64_t* counter, time_t* response_time)
{
	time(response_time);
	*counter = *response_time * 1000;
	return true;
}

