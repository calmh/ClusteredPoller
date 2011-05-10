/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#include "monitor.h"

#include "clbuf.h"
#include "cllog.h"
#include "cltime.h"
#include "globals.h"
#include "multithread.h"
#include "rtgconf.h"
#include "rtgtargets.h"
#include <unistd.h>

static void start_iteration(struct rtgtargets *targets);
static void end_iteration(curms_t interval, curms_t this_iteration);

void *monitor_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct monitor_ctx *monitor_context = (struct monitor_ctx *) thread_context->param;

        struct rtgtargets *targets = monitor_context->targets;
        struct rtgconf *config = monitor_context->config;
        unsigned poll_interval = config->interval;

        curms_t next_iteration = 0;
        curms_t this_iteration = 0;

        struct timespec loopdelay = { 0, 250 * 1000 * 1000 };
        nanosleep(&loopdelay, NULL);

        while (!thread_stop_requested) {
                curms_t now = curms();

                /* If there are no threads running and it's time for a new iteration,
                 * start a new iteration. */
                if (active_threads == 0 && now >= next_iteration) {
                        start_iteration(targets);
                        this_iteration = now;
                        next_iteration = next_interval(now, poll_interval);
                }

                /* Give the threads a chance to start, so we don't
                 * mistakenly think they're all done already. */
                nanosleep(&loopdelay, NULL);

                /* If there are no threads running, and we have started a iteration,
                 * then we are done. End the iteration. */
                if (active_threads == 0 && this_iteration) {
                        end_iteration(next_iteration, this_iteration);
                        this_iteration = 0;
                }
        }

        return NULL;
}

void start_iteration(struct rtgtargets *targets)
{
        statistics.iterations++;

        if (verbosity > 0) {
                unsigned qd = clbuf_count_used(queries);
                cllog(1, "Starting iteration #%d.", statistics.iterations);
                if (qd > 0)
                        cllog(1, "  Queue at depth %d at poll start.", qd);
        }

        pthread_mutex_lock(&global_lock);
        rtgtargets_reset_next(targets);
        pthread_cond_broadcast(&global_cond);
        pthread_mutex_unlock(&global_lock);
}

void end_iteration(curms_t next_iteration, curms_t this_iteration)
{
        curms_t now = curms();
        curms_t to_sleep_ms = next_iteration - now;
        double elapsed = (statistics.query_threads_finished - this_iteration) / 1000.0;
        double to_sleep_s = to_sleep_ms / 1000.0;
        int db_crit = 1;
        int snmp_crit = 1;
        if (statistics.dropped_queries > 0)
                db_crit = 0;
        if (statistics.snmp_fail > 0)
                snmp_crit = 0;

        cllog(1, "Iteration #%d complete.", statistics.iterations);
        cllog(1, " %6.02f seconds elapsed", elapsed);
        cllog(snmp_crit, " %6d SNMP queries made (%.01f queries/s)", statistics.snmp_success + statistics.snmp_fail, (statistics.snmp_success + statistics.snmp_fail) / elapsed);
        cllog(snmp_crit, " %6d of those queries failed", statistics.snmp_fail);
        cllog(db_crit, " %6d database inserts queued (%.01f queries/s)", statistics.insert_queries - statistics.dropped_queries, (statistics.insert_queries - statistics.dropped_queries) / elapsed);
        cllog(db_crit, " %6d inserts were dropped due to lack of buffer space", statistics.dropped_queries);
        cllog(1, " %6d entries maximum queue size", statistics.max_queue_depth);
        if (to_sleep_ms > 0)
                cllog(1, " %6.02f seconds until next iteration", to_sleep_s);

        statistics.insert_rows = 0;
        statistics.insert_queries = 0;
        statistics.snmp_fail = 0;
        statistics.snmp_success = 0;
        statistics.dropped_queries = 0;
        statistics.max_queue_depth = 0;

        if (to_sleep_ms > 0) {
                struct timespec sleep_spec;
                sleep_spec.tv_sec = next_iteration / 1000;
                sleep_spec.tv_nsec = 0;
                pthread_mutex_lock(&global_lock);
                pthread_cond_timedwait(&global_cond, &global_lock, &sleep_spec);
                pthread_mutex_unlock(&global_lock);
        }
}
