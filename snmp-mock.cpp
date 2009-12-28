#include "snmp.h"

#include <string>
#include <iostream>

using namespace std;

unsigned int speed = 1e6 / 8; 

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
	*counter = *response_time * speed;
	return true;
}

void mock_set_speed(unsigned int newspeed)
{
	speed = newspeed;
}

