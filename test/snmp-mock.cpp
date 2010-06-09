#include <string>
#include <iostream>
#include "snmp.h"

using namespace std;

unsigned int speed = 1000000 / 8;

void mock_set_speed(unsigned int newspeed)
{
        speed = newspeed;
}

bool SNMP::global_init_done = false;

SNMP::SNMP(string host, string community, int snmpver)
{
}

SNMP::~SNMP()
{
}

bool SNMP::get_counter(string oid_str, uint64_t* counter, time_t* response_time)
{
        time(response_time);
        *counter = *response_time * speed;
        return true;
}
