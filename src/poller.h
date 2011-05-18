/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#ifndef POLLER_H_
#define POLLER_H_

#include <time.h>

/** @file poller.h SNMP poller thread */

struct rtgtargets;
struct queryhost;

/** Thread context for poller thread. */
struct poller_ctx {
        struct rtgtargets *targets;     /**< The targets to poll. */
};

/**
 * Get all inserts for the specified host for this polling interval.
 * Will cause a lot of SNMP queries etc to happen.
 * @param host The host to query.
 * @return A NULL-terminated list of clinserts.
 */
struct clinsert **get_clinserts(struct queryhost *host);

/**
 * Main loop for the poller thread.
 * @param param A poller_ctx object for this thread.
 */
void *poller_run(void *param);

/**
 * Calculate the rate between two measurements. Correctly handles wrapping counters.
 * @param prev_time Time of previous measurement.
 * @param prev_counter Counter value at previous measurement.
 * @param cur_time Current time.
 * @param cur_counter Current counter value;
 * @param bits Significant number of bits in the counter values (32 or 64).
 * @param [out] counter_diff The difference between the two counters.
 * @param [out] rate The rate, adjusted for time difference.
 */
void calculate_rate(time_t prev_time, unsigned long long prev_counter, time_t cur_time, unsigned long long cur_counter, int bits, unsigned long long *counter_diff, unsigned *rate);

#endif
