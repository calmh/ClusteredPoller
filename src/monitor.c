#include "util.h"
#include "monitor.h"
#include "queryablehost.h"
#include "globals.h"

void *monitor_run(void *ptr)
{
        mt_context *thread_context = (mt_context *) ptr;
        monitor_ctx *monitor_context = (monitor_ctx *) thread_context->param;

        unsigned poll_interval = monitor_context->interval;

        time_t interval = 0;
        time_t iteration_completed = 0;
        time_t in_iteration = 0;
        while (1) {
                sleep(1);
                if (active_threads == 0 && in_iteration) {
                        stat_iterations++;
                        if (verbosity > 0) {
                                unsigned qd = cbuffer_count(queries);
                                cllog(1, "Iteration complete, elapsed time for #%d was %d s.", stat_iterations, time(NULL) - in_iteration);
                                cllog(1, "Time until next iteration is %d s.", interval - time(NULL));
                                cllog(1, "  Rows inserted: %d", stat_inserts);
                                cllog(1, "  Queries queued: %d", stat_queries);
                                cllog(1, "  Max queue depth: %d", query_queue_depth);
                                cllog(1, "  Remaining queue: %d", qd);
                                if (qd > 0)
                                        iteration_completed = time(NULL);
                        }
                        stat_inserts = 0;
                        stat_queries = 0;
                        in_iteration = 0;
                        query_queue_depth = 0;
                }

                if (active_threads == 0 && iteration_completed) {
                        unsigned qd = cbuffer_count(queries);
                        if (qd == 0) {
                                cllog(1,  "  Queue empty %d s after poll completion.", time(NULL) - iteration_completed);
                                iteration_completed = 0;
                        }
                }

                pthread_mutex_lock(&global_lock);
                if (active_threads == 0 && time(NULL) > interval) {
                        interval = (time(NULL) / poll_interval + 1) * poll_interval;
                        in_iteration = time(NULL);
                        if (verbosity >= 1) {
                                cllog(1, "Monitor signals wakeup.");
                                unsigned qd = cbuffer_count(queries);
                                if (qd > 0 && verbosity > 0)
                                        cllog(1, "  Queue at depth %d at poll start.", qd);
                                iteration_completed = 0;
                        }
                        pthread_cond_broadcast(&global_cond);
                }
                pthread_mutex_unlock(&global_lock);
        }
        return NULL;
}

