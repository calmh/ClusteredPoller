#ifndef _RTGTARGETS_H
#define _RTGTARGETS_H

#include <time.h>
#include <pthread.h>
#include "rtgconf.h"

/* Holds query instructions for one row (table+id). */
typedef struct {
        char *oid;
        char *table;
        unsigned id;
        unsigned bits;
        unsigned long long speed;
        unsigned long long cached_counter;
        time_t cached_time;
} queryrow;

queryrow *queryrow_create();
void queryrow_free(queryrow *row);

/* Holds query instructions for one host. */
typedef struct {
        char *host;
        char *community;
        int snmpver;

        queryrow **rows;
        unsigned nrows;
        unsigned allocated_rowspace;
} queryhost;

queryhost *queryhost_create();
void queryhost_free(queryhost *host);

// Holds information from targets.cfg.
typedef struct {
        queryhost **hosts;
        unsigned nhosts;
        unsigned allocated_space;
        unsigned ntargets;
        unsigned next_host;
        pthread_mutex_t next_host_lock;
} rtgtargets;

rtgtargets *rtgtargets_create();
rtgtargets *rtgtargets_parse(const char *filename, const rtgconf *config);
void rtgtargets_free(rtgtargets *targets);
queryhost *rtgtargets_next(rtgtargets *targets);
void rtgtargets_reset_next(rtgtargets *targets);

#endif
