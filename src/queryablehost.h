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

struct db_insert **get_db_inserts(struct queryhost *host);
void db_insert_free(struct db_insert *insert);

#endif
