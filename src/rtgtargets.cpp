#include "rtgtargets.h"
#include "util.h"
#include "globals.h"

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

RTGTargets::RTGTargets()
	: vector<QueryHost>()
{
}

// Simple state machine based config reader.
RTGTargets::RTGTargets(string filename, RTGConf &conf)
	: vector<QueryHost>()
{
	ifstream targets(filename.c_str());
	string token;
	QueryHost host;
	QueryRow row;
	int state = 0;
	int nhosts = 0;
	int ntargs = 0;
	while (targets >> token) {
		token = no_semi(token);
		string_tolower(token);
		if (token[0] == '#') {
			targets.ignore(1024, '\n');
			continue;
		}
		if (state == 0) {
			if (token == "host") {
				host = QueryHost();
				targets >> host.host;
			}
			else if (token == "{")
				state = 1;
		}
		else if (state == 1) {
			if (token == "community") {
				targets >> token;
				host.community = no_semi(token);
			}
			else if (token == "snmpver") {
				targets >> host.snmpver;
			}
			else if (token == "target") {
				targets >> token;
				row = QueryRow();
				row.oid = no_semi(token);
			}
			else if (token == "{") {
				state = 2;
			}
			else if (token == "}") {
				push_back(host);
				nhosts++;
				state = 0;
			}
		}
		else if (state == 2) {
			if (token == "bits")
				targets >> row.bits;
			else if (token == "table") {
				targets >> token;
				row.table = no_semi(token);
			}
			else if (token == "id") {
				targets >> row.id;
			}
			else if (token == "speed") {
				uint64_t max_counter_diff;
				targets >> max_counter_diff;
				if (row.bits == 0)
					row.speed = max_counter_diff;
				else
					row.speed = max_counter_diff / conf.interval;
			}
			else if (token == "}") {
				host.rows.push_back(row);
				ntargs++;
				state = 1;
			}
		}
	}

	targets.close();
	if (verbosity >= 1)
		cerr << "Read " << ntargs << " targets in " << nhosts << " hosts." << endl;
}
