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

// The maximum amount of different tables we expect for a single host,
// i.e. ifInOctets, ifOutOctets, ifInUcastPkts, etc.
#define MAX_TABLES 32

struct queryhost;

// An "insert value", i.e. what will become a table row after inserting into the database.
struct clinsert_value {
        unsigned id;
        unsigned long long counter;
        unsigned rate;
        time_t dtime;
};

// An "insert", i.e. what will become a SQL INSERT query.
struct clinsert {
        char *table;
        struct clinsert_value *values;
        unsigned nvalues;
        unsigned allocated_space;
};

// Get a clinsert object for use with the specified table.
// Will return an existing one if there is one in **inserts, or a new one will be
// created an added to the list **inserts.
struct clinsert *clinsert_for_table(struct clinsert **inserts, char *table);

// Get all inserts for the specified host for this polling interval.
// Will cause a lot of SNMP queries etc to happen.
struct clinsert **get_clinserts(struct queryhost *host);

// Free a clinsert object.
void clinsert_free(struct clinsert *insert);

// Push a new value to the clinsert.
// Will create a clinsert_value.
void clinsert_push_value(struct clinsert *insert, unsigned id, unsigned long long counter, unsigned rate, time_t dtime);

// Count the number of clinserts in the list **inserts.
unsigned clinsert_count(struct clinsert **inserts);

#endif /* CLINSERT_H */