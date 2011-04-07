#define _GNU_SOURCE

#include <string.h>
#include <time.h>

#include "clbuf.h"
#include "multithread.h"
#include "globals.h"
#include "poller.h"
#include "clsnmp.h"
#include "queryablehost.h"
#include "cllog.h"
#include "rtgtargets.h"

void *poller_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct poller_ctx *poller_context = (struct poller_ctx *) thread_context->param;

        unsigned id = thread_context->thread_id;
        struct rtgtargets *targets = poller_context->targets;

        // Start looping.
        unsigned iterations = 0;
        time_t start = 0, end = 0;
        while (!thread_stop_requested) {

                // Mark ourself sleeping
                if (iterations > 0)
                        cllog(2, "Thread %d sleeping after %d s processing time.", id, end - start);

                // Wait for green light.
                pthread_mutex_lock(&global_lock);
                if (iterations > 0)
                        active_threads--;
                if (!active_threads)    // We are the last one
                        gettimeofday(&query_threads_finished, NULL);
                pthread_cond_wait(&global_cond, &global_lock);
                pthread_mutex_unlock(&global_lock);

                if (thread_stop_requested)
                        break;

                cllog(2, "Thread %d starting.", id);
                // Mark ourself active.
                pthread_mutex_lock(&global_lock);
                active_threads++;
                pthread_mutex_unlock(&global_lock);

                // Note our start time, so we know how long an iteration takes.
                start = time(NULL);
                // Loop over our share of the hosts.
                struct queryhost *host;
                unsigned dropped_queries = 0;
                unsigned queued_queries = 0, queued_values = 0;
                while (!thread_stop_requested && (host = rtgtargets_next(targets))) {
                        cllog(2, "Thread %d picked host '%s'.", id, host->host);
                        // Process the host and get back a list of SQL updates to execute.
                        struct db_insert **host_queries = get_db_inserts(host);
                        unsigned n_queries;
                        for (n_queries = 0; host_queries[n_queries]; n_queries++) ;

                        if (n_queries > 0) {
                                cllog(2, "Thread %u queueing %u queries.", id, n_queries);

                                unsigned i;
                                for (i = 0; i < n_queries && clbuf_count_free(queries) > 0; i++) {
                                        void *result = clbuf_push(queries, host_queries[i]);
                                        if (result) {
                                                queued_queries++;
                                                queued_values += host_queries[i]->nvalues;
                                        } else {
                                                break;
                                        }
                                }
                                unsigned qd = clbuf_count_used(queries);
                                query_queue_depth = query_queue_depth > qd ? query_queue_depth : qd;
                                if (i != n_queries) {
                                        if (!dropped_queries)
                                                cllog(0, "Thread %d dropped queries due to database queue full.", id);
                                        dropped_queries += n_queries - i;
                                }
                        }
                        free(host_queries);
                }

                pthread_mutex_lock(&global_lock);
                stat_inserts += queued_values;
                stat_queries += queued_queries;
                stat_dropped_queries += dropped_queries;
                pthread_mutex_unlock(&global_lock);

                // Note how long it took.
                end = time(NULL);

                // Prepare for next iteration.
                iterations++;
        }
        return NULL;
}
