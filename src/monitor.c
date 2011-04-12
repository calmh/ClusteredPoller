#include <sys/time.h>
#include <unistd.h>

#include "clbuf.h"
#include "cllog.h"
#include "monitor.h"
#include "globals.h"
#include "rtgtargets.h"
#include "multithread.h"
#include "rtgconf.h"

void *monitor_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct monitor_ctx *monitor_context = (struct monitor_ctx *) thread_context->param;

        unsigned poll_interval = monitor_context->interval;
        struct rtgtargets *targets = monitor_context->targets;
        struct rtgconf *config = monitor_context->config;

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
                                double elapsed = statistics.query_threads_finished.tv_sec - iteration_started.tv_sec + (statistics.query_threads_finished.tv_usec - iteration_started.tv_usec) / 1e6;
                                double to_sleep = interval - now.tv_sec - now.tv_usec / 1e6;
                                cllog(1, "Iteration #%d complete.", statistics.iterations);
                                cllog(1, " %6.02f seconds elapsed", elapsed);
                                cllog(1, " %6d SNMP queries made (%.01f queries/s)", statistics.snmp_success + statistics.snmp_fail, (statistics.snmp_success + statistics.snmp_fail) / elapsed);
                                cllog(1, " %6d of those queries failed", statistics.snmp_fail);
                                cllog(1, " %6d database inserts queued (%.01f queries/s)", statistics.insert_queries - statistics.dropped_queries, (statistics.insert_queries - statistics.dropped_queries) / elapsed);
                                cllog(1, " %6d inserts were dropped due to lack of buffer space", statistics.dropped_queries);
                                cllog(1, " %6d entries maximum queue size (%.01f %% full)", statistics.max_queue_depth, 100.0 * statistics.max_queue_depth / config->max_db_queue);
                                cllog(1, " %6.02f seconds until next iteration", to_sleep);
                        }

                        statistics.insert_rows = 0;
                        statistics.insert_queries = 0;
                        statistics.snmp_fail = 0;
                        statistics.snmp_success = 0;
                        statistics.dropped_queries = 0;
                        statistics.max_queue_depth = 0;
                        in_iteration = 0;

                        struct timespec sleep_spec = { interval - now.tv_sec - 1, 1000000000l - now.tv_usec * 1000 };
                        pthread_mutex_lock(&global_lock);
                        pthread_cond_timedwait(&global_cond, &global_lock, &sleep_spec);
                        pthread_mutex_unlock(&global_lock);
                }

                gettimeofday(&now, NULL);
                if (active_threads == 0 && now.tv_sec >= interval) {
                        interval = (now.tv_sec / poll_interval + 1) * poll_interval;
                        in_iteration = 1;
                        statistics.iterations++;

                        cllog(1, "Starting iteration #%d.", statistics.iterations);
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
