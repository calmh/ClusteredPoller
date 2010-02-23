#ifndef _RTGCONF_H
#define _RTGCONF_H

#include <string>

// Holds information from rtg.conf.
struct RTGConf {
	RTGConf();
	RTGConf(std::string filename);

	unsigned interval;
	unsigned threads;
	double high_skew_slop;
	double low_skew_slop;
	std::string dbhost;
	std::string database;
	std::string dbuser;
	std::string dbpass;
};

#endif
