//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#ifndef CLINSERT_H
#define CLINSERT_H

/// @file clinsert.h
/// A database insert representation used internally in clpoll.

#include <time.h>

/// The maximum amount of different tables we expect for a single host,
/// i.e. ifInOctets, ifOutOctets, ifInUcastPkts, etc.
#define MAX_TABLES 32

struct queryhost;

/// An "insert value", what will become a table row after inserting into the database.
/// @see clinsert_for_table
/// @see clinsert_push_value
struct clinsert_value {
        unsigned id;
        unsigned long long counter;
        unsigned rate;
        time_t dtime;
};

/// An "insert", what will become a SQL INSERT query.
/// @see clinsert_for_table
/// @see clinsert_push_value
/// @see clinsert_count
/// @see clinsert_free
struct clinsert {
        char *table;
        struct clinsert_value *values;
        unsigned nvalues;
        unsigned allocated_space;
};

/// Get a clinsert object for use with the specified table.
/// Will create at most MAX_TABLES objects in the list **inserts, and assumes that the list can actually hold this meny objects.
/// @param inserts A list of inserts to either find an existing object in, or insert a new one into.
/// @return An existing object if there is one in **inserts, or a new one will be created an added to the list **inserts.
struct clinsert *clinsert_for_table(struct clinsert **inserts, char *table);

// Free a clinsert object.
void clinsert_free(struct clinsert *insert);

// Push a new value to the clinsert.
// Will create a clinsert_value.
void clinsert_push_value(struct clinsert *insert, unsigned id, unsigned long long counter, unsigned rate, time_t dtime);

// Count the number of clinserts in the list **inserts.
unsigned clinsert_count(struct clinsert **inserts);

#endif /* CLINSERT_H */
