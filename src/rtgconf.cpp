#include <fstream>
#include "rtgconf.h"
#include "util.h"

using namespace std;

RTGConf::RTGConf()
{
	interval = 300;
	threads = 2;
}

RTGConf::RTGConf(string filename)
{
	ifstream rtgconf(filename.c_str());
	string token;
	while (rtgconf >> token) {
		string_tolower(token);
		if (token == "interval")
			rtgconf >> interval;
		else if (token == "highskewslop") // Not sure what these are, possibly something for rtgplot to determine when data is too old to plot etc?
			rtgconf >> high_skew_slop;
		else if (token == "lowskewslop") // Not sure what these are, possibly something for rtgplot to determine when data is too old to plot etc?
			rtgconf >> low_skew_slop;
		else if (token == "db_host")
			rtgconf >> dbhost;
		else if (token == "db_database")
			rtgconf >> database;
		else if (token == "db_user")
			rtgconf >> dbuser;
		else if (token == "db_pass")
			rtgconf >> dbpass;
		else if (token == "threads")
			rtgconf >> threads;
	}
}

