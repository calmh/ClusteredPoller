#ifndef _SNMP_H
#define _SNMP_H

#include <string>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

class SNMP {
private:
	static bool global_init_done;
	struct snmp_session session;
	void *sessp;

public:
	SNMP(std::string host, std::string community);
	~SNMP();
	bool get_counter(std::string oid_str, uint64_t* counter, time_t* response_time);
};

#endif

