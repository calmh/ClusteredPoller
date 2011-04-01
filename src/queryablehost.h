#ifndef _QUERYABLEHOST_H
#define _QUERYABLEHOST_H

#include <time.h>
#include "snmp.h"
#include "rtgtargets.h"

#ifdef __cplusplus
extern "C" {
#endif

        typedef struct {
                unsigned id;
                unsigned long long counter;
                unsigned rate;
                time_t dtime;
        } db_insert_value;

        typedef struct {
                char *table;
                db_insert_value *values;
                unsigned nvalues;
                unsigned allocated_space;
        } db_insert;

        char **get_inserts(queryhost *host);
        void calculate_rate(time_t prev_time, unsigned long long prev_counter, time_t cur_time, unsigned long long cur_counter, int bits, unsigned long long *counter_diff, unsigned *rate);
        db_insert **get_db_inserts(queryhost *host);

#ifdef __cplusplus
}
#endif

#endif
