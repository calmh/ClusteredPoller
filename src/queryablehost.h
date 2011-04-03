#ifndef _QUERYABLEHOST_H
#define _QUERYABLEHOST_H

#include <time.h>

struct clsnmp_session;
struct queryhost;

struct db_insert_value {
        unsigned id;
        unsigned long long counter;
        unsigned rate;
        time_t dtime;
};

struct db_insert {
        char *table;
        struct db_insert_value *values;
        unsigned nvalues;
        unsigned allocated_space;
};

char **get_inserts(struct queryhost *host);
void calculate_rate(time_t prev_time, unsigned long long prev_counter, time_t cur_time, unsigned long long cur_counter, int bits, unsigned long long *counter_diff, unsigned *rate);
struct db_insert **get_db_inserts(struct queryhost *host);

#endif
