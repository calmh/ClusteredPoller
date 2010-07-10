#ifndef _TYPES_H
#define _TYPES_H

#include <map>
#include <vector>
#include <string>

//
// Cache stuff.
//

// Holds cache data for one host.
struct ResultCache {
        std::map<std::pair<std::string, int>, unsigned long long> counters;
        std::map<std::pair<std::string, int>, time_t> times;
};

//
// Result stuff.
//

// Holds result data for one row (table+id).
struct ResultRow {
        int id;
        unsigned long long counter;
        unsigned long long rate;
        int bits;
        time_t dtime;
        unsigned long long speed;

        ResultRow(int iid, unsigned long long icounter, unsigned long long irate, int ibits, time_t idtime, unsigned long long ispeed) {
                id = iid;
                counter = icounter;
                rate = irate;
                bits = ibits;
                dtime = idtime;
                speed = ispeed;
        }
};

// Holds result data for one host.
struct ResultSet {
        std::string table;
        std::vector<ResultRow> rows;

        ResultSet() {}
        ResultSet(std::string itable) {
                table = itable;
        }
};

//
// Query instructions stuff.
//

#endif
