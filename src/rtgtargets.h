#ifndef _RTGTARGETS_H
#define _RTGTARGETS_H

#include <time.h>
#include <pthread.h>

struct rtgconf;

/* Holds query instructions for one row (table+id). */
struct queryrow {
        char *oid;
        char *table;
        unsigned id;
        unsigned bits;
        unsigned long long speed;
        unsigned long long cached_counter;
        time_t cached_time;
};

struct queryrow *queryrow_create();
void queryrow_free(struct queryrow *row);

/* Holds query instructions for one host. */
struct queryhost {
        char *host;
        char *community;
        int snmpver;

        struct queryrow **rows;
        unsigned nrows;
        unsigned allocated_rowspace;
};

struct queryhost *queryhost_create();
void queryhost_free(struct queryhost *host);

// Holds information from targets.cfg.
struct rtgtargets {
        struct queryhost **hosts;
        unsigned nhosts;
        unsigned allocated_space;
        unsigned ntargets;
        unsigned next_host;
        pthread_mutex_t next_host_lock;
};

struct rtgtargets *rtgtargets_create();
struct rtgtargets *rtgtargets_parse(const char *filename, const struct rtgconf *config);
void rtgtargets_free(struct rtgtargets *targets);
struct queryhost *rtgtargets_next(struct rtgtargets *targets);
void rtgtargets_reset_next(struct rtgtargets *targets);

#endif
