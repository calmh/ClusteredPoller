/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#ifndef RTGTARGETS_H
#define RTGTARGETS_H

/** @file rtgtargets.h RTG targets file parsing */

#include <time.h>
#include <pthread.h>

struct rtgconf;

/**
 * Object to hold query instructions for one row (table/id combination).
 * @see queryrow_create
 * @see queryrow_free
 * @see queryhost
 */
struct queryrow {
        char *oid;              /**< The SNMP OID to query. */
        char *table;            /**< The database table to insert the result into. */
        unsigned id;            /**< The ID to use. */
        unsigned bits;          /**< The number of bits (0=gauge, 32, 64). */
        unsigned long long speed;       /**< The speed of this interface, in bytes/s. */
        unsigned long long cached_counter;      /**< The counter value we saw last polling round. */
        time_t cached_time;     /**< The time value of the last polling round. */
};

/**
 * Create a new queryrow object.
 * @return A new queryrow object.
 */
struct queryrow *queryrow_create(void);

/**
 * Free a queryrow object.
 * @param row The queryrow to free.
 */
void queryrow_free(struct queryrow *row);

/**
 * Object to hold query instructions for one host.
 * @see queryhost_create
 * @see queryhost_free
 * @see rtgtargets_next
 */
struct queryhost {
        char *host;             /**< Host name or IP. */
        char *community;        /**< SNMP community name. */
        int snmpver;            /**< SNMP version, 1 or 2 (for 2c). */

        struct queryrow **rows; /**< List of queryrows. */
        unsigned nrows;         /**< Number of queryrows in rows. */
        unsigned allocated_rowspace;    /**< Allocated size of the rows list. */
};

/**
 * Create a new queryhost object.
 * @return A new queryhost object.
 */
struct queryhost *queryhost_create(void);

/**
 * Free a queryhost object.
 * @param host The object to free.
 */
void queryhost_free(struct queryhost *host);

/**
 * Holds information from targets.cfg.
 * @see rtgtargets_create
 * @see rtgtargets_parse
 * @see rtgtargets_free
 * @see rtgtargets_next
 * @see rtgtargets_reset_next
 */
struct rtgtargets {
        struct queryhost **hosts;       /**< List of queryhosts. */
        unsigned nhosts;        /**< The number of queryhosts in hosts. */
        unsigned allocated_space;       /**< Allocated size of the hosts list. */
        unsigned ntargets;      /**< Total number of targets (queryrows). */
        unsigned next_host;     /**< Next host to poll. */
        pthread_mutex_t next_host_lock; /**< Lock for getting next host to poll. */
};

/**
 * Create a new rtgtargets object.
 * @return A new rtgtargets object.
 */
struct rtgtargets *rtgtargets_create(void);

/**
 * Parse a targets file, with extra parameters from the config object.
 * @param filename The name of the targets file.
 * @param config The parsed rtg.conf file.
 * @return A new rtgtargets object.
 */
struct rtgtargets *rtgtargets_parse(const char *filename, const struct rtgconf *config);

/**
 * Free a rtgtargets object.
 * @param targets The rtgtargets object to free.
 */
void rtgtargets_free(struct rtgtargets *targets);

/**
 * Get the next queryhost for polling.
 * @param targets The targets object to get a queryhost from.
 * @return A queryhost object to poll, or NULL of there are no more targets to poll this round.
 */
struct queryhost *rtgtargets_next(struct rtgtargets *targets);

/**
 * Reset the next_host pointer to prepare for a new polling round.
 * @param targets The rtgtargets object which should be reset.
 */
void rtgtargets_reset_next(struct rtgtargets *targets);

#endif /* RTGTARGETS_H */
