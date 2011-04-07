#ifndef _RTGCONF_H
#define _RTGCONF_H

struct rtgconf {
        // Standard RTG
        unsigned interval;
        unsigned threads;
        char *dbhost;
        char *database;
        char *dbuser;
        char *dbpass;
        // Extended
        int use_db;
        int use_rate_column;
        int allow_db_zero;
        unsigned dbthreads_divisor;
        unsigned max_db_queue;
};

struct rtgconf *rtgconf_create(const char *filename);
int rtgconf_verify(struct rtgconf *config);
void rtgconf_free(struct rtgconf *config);

#endif
