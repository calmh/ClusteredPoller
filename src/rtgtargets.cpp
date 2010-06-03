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
RTGTargets::RTGTargets(string filename, RTGConf& conf)
        : vector<QueryHost>()
{
        int hosts_read = read_new_style_targets(filename, conf);
        if (hosts_read == 0) {
                read_old_style_targets(filename, conf);
        }
}

int RTGTargets::read_new_style_targets(string filename, RTGConf& conf)
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
                        } else if (token == "{")
                                state = 1;
                } else if (state == 1) {
                        if (token == "community") {
                                targets >> token;
                                host.community = no_semi(token);
                        } else if (token == "snmpver") {
                                targets >> host.snmpver;
                        } else if (token == "target") {
                                targets >> token;
                                row = QueryRow();
                                row.oid = no_semi(token);
                        } else if (token == "{") {
                                state = 2;
                        } else if (token == "}") {
                                push_back(host);
                                nhosts++;
                                state = 0;
                        }
                } else if (state == 2) {
                        if (token == "bits")
                                targets >> row.bits;
                        else if (token == "table") {
                                targets >> token;
                                row.table = no_semi(token);
                        } else if (token == "id") {
                                targets >> row.id;
                        } else if (token == "speed") {
                                uint64_t max_counter_diff;
                                targets >> max_counter_diff;
                                if (row.bits == 0)
                                        row.speed = max_counter_diff;
                                else
                                        row.speed = max_counter_diff / conf.interval;
                        } else if (token == "}") {
                                bool duplicate = false;
                                for (vector<QueryRow>::const_iterator it = host.rows.begin(); it != host.rows.end(); it++) {
                                        if (it->oid.compare(row.oid) == 0) {
                                                duplicate = true;
                                                cerr << "WARNING: Host " << host.host << " OID " << row.oid << " is a duplicate. Ignoring." << endl;
                                                break;
                                        }
                                        if (it->table.compare(row.table) == 0 && it->id == row.id) {
                                                duplicate = true;
                                                cerr << "WARNING: Host " << host.host << " table " << row.table << " id " << row.id << " is a duplicate. Ignoring." << endl;
                                                break;
                                        }
                                }
                                if (!duplicate) {
                                        host.rows.push_back(row);
                                        ntargs++;
                                }
                                state = 1;
                        }
                }
        }

        targets.close();
        if (verbosity >= 1)
                cerr << "Read " << ntargs << " targets in " << nhosts << " hosts." << endl;
        return nhosts;
}

int RTGTargets::read_old_style_targets(string filename, RTGConf& conf)
{
        ifstream targets(filename.c_str());
        char linebuffer[256];

        int nhosts = 0;
        QueryHost* currentHost = NULL;
        while (!targets.eof()) {
                targets.getline(linebuffer, 255);
                string line(linebuffer);

                line = string_uncomment(line);
                if (line.length() == 0)
                        continue;

                list<string> parts = string_split(line, "\t");
                if (parts.size() < 6)
                        continue;

                string host = parts.front();
                parts.pop_front();
                string oid = parts.front();
                parts.pop_front();
                int bits = atoi(parts.front().c_str());
                parts.pop_front();
                string community = parts.front();
                parts.pop_front();
                string table = parts.front();
                parts.pop_front();
                int id = atoi(parts.front().c_str());
                parts.pop_front();

                if (currentHost != NULL && currentHost->host.compare(host) != 0) {
                        push_back(*currentHost);
                        nhosts++;
                        currentHost = NULL;
                }

                if (currentHost == NULL) {
                        // We lack data, so we assume SNMP version 2.
                        currentHost = new QueryHost(host, community, 2);
                };

                QueryRow row(oid, table, id, bits);
                // We lack data so we assume a tengig interface.
                row.speed = (unsigned)10e9 / 8 / conf.interval;
                currentHost->rows.push_back(row);
        }

        push_back(*currentHost);
        nhosts++;

        return nhosts;
}
