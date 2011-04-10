//
//  clinsert.h
//  ClusteredPoller
//
//  Created by Jakob Borg on 2011-04-10.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#ifndef CLINSERT_H
#define CLINSERT_H

#include <time.h>

#define MAX_TABLES 32

struct queryhost;

struct clinsert_value {
        unsigned id;
        unsigned long long counter;
        unsigned rate;
        time_t dtime;
};

struct clinsert {
        char *table;
        struct clinsert_value *values;
        unsigned nvalues;
        unsigned allocated_space;
};

struct clinsert **get_clinserts(struct queryhost *host);
void clinsert_free(struct clinsert *insert);
void clinsert_push_value(struct clinsert *insert, unsigned id, unsigned long long counter, unsigned rate, time_t dtime);
struct clinsert *clinsert_for_table(struct clinsert **inserts, char *table);
unsigned clinsert_count(struct clinsert **inserts);

#endif /* CLINSERT_H */