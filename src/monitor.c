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

void *monitor_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct monitor_ctx *monitor_context = (struct monitor_ctx *) thread_context->param;

        struct rtgtargets *targets = monitor_context->targets;
        struct rtgconf *config = monitor_context->config;
        unsigned poll_interval = config->interval;

        int in_iteration = 0;
        curms_t interval = 0;
        curms_t iteration_started = 0;
        curms_t now = 0;

        struct timespec loopdelay = { 0, 250 * 1000 * 1000 };
        nanosleep(&loopdelay, NULL);

        while (!thread_stop_requested) {
                struct timespec sleep_spec;

                now = curms();
                if (active_threads == 0 && in_iteration) {
                        curms_t to_sleep_ms = interval - now;
                        if (verbosity > 0) {
                                double elapsed = (statistics.query_threads_finished - iteration_started) / 1000.0;
                                double to_sleep_s = to_sleep_ms / 1000.0;
                                cllog(1, "Iteration #%d complete.", statistics.iterations);
                                cllog(1, " %6.02f seconds elapsed", elapsed);
                                cllog(1, " %6d SNMP queries made (%.01f queries/s)", statistics.snmp_success + statistics.snmp_fail, (statistics.snmp_success + statistics.snmp_fail) / elapsed);
                                cllog(1, " %6d of those queries failed", statistics.snmp_fail);
                                cllog(1, " %6d database inserts queued (%.01f queries/s)", statistics.insert_queries - statistics.dropped_queries, (statistics.insert_queries - statistics.dropped_queries) / elapsed);
                                cllog(1, " %6d inserts were dropped due to lack of buffer space", statistics.dropped_queries);
                                cllog(1, " %6d entries maximum queue size (%.01f %% full)", statistics.max_queue_depth, 100.0 * statistics.max_queue_depth / config->max_db_queue);
                                if (to_sleep_ms > 0)
                                        cllog(1, " %6.02f seconds until next iteration", to_sleep_s);
                        }

                        statistics.insert_rows = 0;
                        statistics.insert_queries = 0;
                        statistics.snmp_fail = 0;
                        statistics.snmp_success = 0;
                        statistics.dropped_queries = 0;
                        statistics.max_queue_depth = 0;
                        in_iteration = 0;

                        if (to_sleep_ms > 0) {
                                sleep_spec.tv_sec = (interval - now) / 1000;
                                sleep_spec.tv_nsec = ((interval - now) % 1000) * 1000000;
                                pthread_mutex_lock(&global_lock);
                                pthread_cond_timedwait(&global_cond, &global_lock, &sleep_spec);
                                pthread_mutex_unlock(&global_lock);
                        }
                }

                now = curms();
                if (active_threads == 0 && now >= interval) {
                        unsigned qd;

                        interval = 1000 * ((now / 1000) / poll_interval + 1) * poll_interval;
                        in_iteration = 1;
                        statistics.iterations++;

                        cllog(1, "Starting iteration #%d.", statistics.iterations);
                        qd = clbuf_count_used(queries);
                        if (qd > 0)
                                cllog(1, "  Queue at depth %d at poll start.", qd);

                        pthread_mutex_lock(&global_lock);
                        rtgtargets_reset_next(targets);
                        pthread_cond_broadcast(&global_cond);
                        pthread_mutex_unlock(&global_lock);
                        iteration_started = curms();
                }

                nanosleep(&loopdelay, NULL);
        }
        return NULL;
}
