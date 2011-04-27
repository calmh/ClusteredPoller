/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#ifndef CLINSERT_H
#define CLINSERT_H

#include <time.h>

/** @file clinsert.h A database insert representation used internally in clpoll. */

/**
 * The maximum amount of different tables we expect for a single host,
 * i.e. ifInOctets, ifOutOctets, ifInUcastPkts, etc.
 */
#define MAX_TABLES 32

struct queryhost;

/**
 * An "insert value", what will become a table row after inserting into the database.
 * @see clinsert_for_table
 * @see clinsert_push_value
 */
struct clinsert_value {
        unsigned id;            /**< ID value. */
        unsigned long long counter;     /**< Counter value. */
        unsigned rate;          /**< Rate value (calculated form counter and time difference). */
        time_t dtime;           /**< Timestamp value. */
};

/**
 * An "insert", what will become a SQL INSERT query.
 * @see clinsert_for_table
 * @see clinsert_push_value
 * @see clinsert_count
 * @see clinsert_free
 */
struct clinsert {
        char *table;            /**< Table to insert values into. */
        struct clinsert_value *values;  /**< List of values to insert. */
        unsigned nvalues;       /**< Number of values in values list. */
        unsigned allocated_space;       /**< Allocated size of values list. */
};

/**
 * Get a clinsert object for use with the specified table.
 * Will create at most MAX_TABLES objects in the list **inserts, and assumes that the list can actually hold this meny objects.
 * @param inserts A list of inserts to either find an existing object in, or insert a new one into.
 * @param table Name of the table to find or create an insert for.
 * @return An existing object if there is one in **inserts, or a new one will be created an added to the list **inserts.
 */
struct clinsert *clinsert_for_table(struct clinsert **inserts, char *table);

/**
 * Free a clinsert object.
 * @param insert The clinsert object to free.
 */
void clinsert_free(struct clinsert *insert);

/**
 * Push a new value to the clinsert.
 * Will create a clinsert_value.
 * @param insert The insert to push a new value into.
 * @param id ID value.
 * @param counter Counter value.
 * @param rate Rate value.
 * @param dtime Timestamp value.
 */
void clinsert_push_value(struct clinsert *insert, unsigned id, unsigned long long counter, unsigned rate, time_t dtime);

/**
 * Count the number of clinserts in the list **inserts.
 * @param inserts List of inserts.
 * @return The number of inserts in the list.
 */
unsigned clinsert_count(struct clinsert **inserts);

#endif /* CLINSERT_H */
