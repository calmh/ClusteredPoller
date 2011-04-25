/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#ifndef POLLER_H_
#define POLLER_H_

/** @file poller.h SNMP poller thread */

#include <time.h>

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

#endif
