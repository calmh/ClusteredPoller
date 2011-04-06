#include <unistd.h>

#include "clbuf.h"
#include "cllog.h"
#include "monitor.h"
#include "queryablehost.h"
#include "globals.h"
#include "rtgtargets.h"
#include "multithread.h"

void *monitor_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct monitor_ctx *monitor_context = (struct monitor_ctx *) thread_context->param;

        unsigned poll_interval = monitor_context->interval;
        struct rtgtargets *targets = monitor_context->targets;

        time_t interval = 0;
        time_t iteration_completed = 0;
        time_t in_iteration = 0;
        while (!thread_stop_requested) {
                sleep(1);
                if (active_threads == 0 && in_iteration) {
                        stat_iterations++;
                        if (verbosity > 0) {
                                unsigned qd = clbuf_count_used(queries);
                                time_t now = time(NULL);
                                time_t elapsed = now - in_iteration;
                                cllog(1, "Iteration #%d complete.", stat_iterations);
                                cllog(1, "  Elapsed time:      %4d s.", elapsed);
                                cllog(1, "  Next iteration in: %4d s.", interval - now);
                                if (elapsed > 0) {
                                        cllog(1, "  Queries total:   %6d (%5d queries/s)", (stat_snmp_success+stat_snmp_fail), (stat_snmp_success+stat_snmp_fail)/elapsed);
                                        cllog(1, "  Queries failed:  %6d (%5d queries/s)", stat_snmp_fail, stat_snmp_fail/elapsed);
                                        cllog(1, "  Inserts Queued:  %6d (%5d queries/s)", (stat_queries-stat_dropped_queries), (stat_queries-stat_dropped_queries)/elapsed);
                                }
                                cllog(1, "  Inserts Dropped: %6d", stat_dropped_queries);
                                cllog(1, "  Max Queue Depth:  %5d (%4d%% of max)", query_queue_depth, 100*query_queue_depth/max_queue_length);
                                cllog(1, "  Remaining Queue:  %5d", qd);
                                if (qd > 0)
                                        iteration_completed = now;
                        }
                        stat_inserts = 0;
                        stat_queries = 0;
                        stat_snmp_fail = 0;
                        stat_snmp_success = 0;
                        stat_dropped_queries = 0;
                        in_iteration = 0;
                        query_queue_depth = 0;
                }

                if (active_threads == 0 && iteration_completed) {
                        unsigned qd = clbuf_count_used(queries);
                        if (qd == 0) {
                                cllog(1,  "Queue empty %d s after poll completion.", time(NULL) - iteration_completed);
                                iteration_completed = 0;
                        }
                }

                pthread_mutex_lock(&global_lock);
                if (active_threads == 0 && time(NULL) > interval) {
                        interval = (time(NULL) / poll_interval + 1) * poll_interval;
                        in_iteration = time(NULL);
                        if (verbosity >= 1) {
                                cllog(1, "Monitor signals wakeup.");
                                unsigned qd = clbuf_count_used(queries);
                                if (qd > 0 && verbosity > 0)
                                        cllog(1, "  Queue at depth %d at poll start.", qd);
                                iteration_completed = 0;
                        }
                        rtgtargets_reset_next(targets);
                        pthread_cond_broadcast(&global_cond);
                }
                pthread_mutex_unlock(&global_lock);
        }
        return NULL;
}

