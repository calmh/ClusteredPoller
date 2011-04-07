#include <sys/time.h>
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
        int in_iteration = 0;
        struct timeval iteration_started = { 0 };
        struct timeval now = { 0 };

        struct timespec loopdelay = { 0, 250 * 1000 * 1000 };
        nanosleep(&loopdelay, NULL);

        while (!thread_stop_requested) {
                gettimeofday(&now, NULL);
                if (active_threads == 0 && in_iteration) {
                        if (verbosity > 0) {
                                double elapsed = query_threads_finished.tv_sec - iteration_started.tv_sec + (query_threads_finished.tv_usec - iteration_started.tv_usec) / 1e6;
                                double to_sleep = interval - now.tv_sec - now.tv_usec / 1e6;
                                cllog(1, "Iteration #%d complete.", stat_iterations);
                                cllog(1, " %6.02f seconds elapsed", elapsed);
                                cllog(1, " %6d SNMP queries made (%.01f queries/s)", stat_snmp_success + stat_snmp_fail, (stat_snmp_success + stat_snmp_fail) / elapsed);
                                cllog(1, " %6d of those queries failed", stat_snmp_fail);
                                cllog(1, " %6d database inserts queued (%.01f queries/s)", stat_queries - stat_dropped_queries, (stat_queries - stat_dropped_queries) / elapsed);
                                cllog(1, " %6d inserts were dropped due to lack of buffer space", stat_dropped_queries);
                                cllog(1, " %6d entries maximum queue size (%.01f %% full)", query_queue_depth, 100.0 * query_queue_depth / max_queue_length);
                                cllog(1, " %6.02f seconds until next iteration", to_sleep);
                        }

                        stat_inserts = 0;
                        stat_queries = 0;
                        stat_snmp_fail = 0;
                        stat_snmp_success = 0;
                        stat_dropped_queries = 0;
                        in_iteration = 0;
                        query_queue_depth = 0;

                        struct timespec sleep_spec = { interval - now.tv_sec - 1, 1000000000l - now.tv_usec * 1000 };
                        nanosleep(&sleep_spec, NULL);
                }

                gettimeofday(&now, NULL);
                if (active_threads == 0 && now.tv_sec >= interval) {
                        interval = (now.tv_sec / poll_interval + 1) * poll_interval;
                        in_iteration = 1;
                        stat_iterations++;

                        cllog(1, "Starting iteration #%d.", stat_iterations);
                        unsigned qd = clbuf_count_used(queries);
                        if (qd > 0)
                                cllog(1, "  Queue at depth %d at poll start.", qd);

                        pthread_mutex_lock(&global_lock);
                        rtgtargets_reset_next(targets);
                        pthread_cond_broadcast(&global_cond);
                        pthread_mutex_unlock(&global_lock);
                        gettimeofday(&iteration_started, NULL);
                }

                nanosleep(&loopdelay, NULL);
        }
        return NULL;
}
