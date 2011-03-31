#ifndef _RTGTARGETS_H
#define _RTGTARGETS_H

#include <string>
#include <vector>
#include "rtgconf.h"

// Holds query instructions for one row (table+id).
struct QueryRow {
        std::string oid;
        std::string table;
        unsigned id;
        unsigned bits;
        unsigned long long speed;

        QueryRow() {}
        QueryRow(std::string ioid, std::string itable, int iid, int ibits) {
                oid = ioid;
                table = itable;
                id = iid;
                bits = ibits;
        }
};

// Holds query instructions for one host.
struct QueryHost {
        std::string host;
        std::string community;
        int snmpver;
        std::vector<QueryRow> rows;

        QueryHost() {
                host = "none";
        }
        QueryHost(std::string ihost, std::string icommunity, int isnmpver) {
                host = ihost;
                community = icommunity;
                snmpver = isnmpver;
        }
};

// Results from targets file reading
struct ParseResults {
        unsigned hosts;
        unsigned targets;
};

// Holds information from targets.cfg.
class RTGTargets : public std::vector<QueryHost>
{
public:
        RTGTargets();
        RTGTargets(std::string filename, rtgconf* config);

        unsigned interval;
        unsigned threads;
        double high_skew_slop;
        double low_skew_slop;
        std::string dbhost;
        std::string database;
        std::string dbuser;
        std::string dbpass;
        ParseResults results;

private:
        QueryHost read_host(std::ifstream& targets, std::string& host_name, rtgconf* conf);
        QueryRow read_row(std::ifstream& targets, std::string& oid, rtgconf* conf);
        bool check_for_duplicate(QueryHost& host, QueryRow& row);
        ParseResults read_new_style_targets(std::string filename, rtgconf* conf);
        ParseResults read_old_style_targets(std::string filename, rtgconf* conf);
};

#endif
