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
        std::map<std::pair<std::string, int>, uint64_t> counters;
        std::map<std::pair<std::string, int>, time_t> times;
};

//
// Result stuff.
//

// Holds result data for one row (table+id).
struct ResultRow {
        int id;
        uint64_t counter;
        uint64_t rate;
        int bits;
        time_t dtime;
        uint64_t speed;

        ResultRow(int iid, uint64_t icounter, uint64_t irate, int ibits, time_t idtime, uint64_t ispeed) {
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
