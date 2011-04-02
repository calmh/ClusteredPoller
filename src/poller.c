#include "multithread.h"
#include "globals.h"
#include "poller.h"
#include "snmp.h"
#include "queryablehost.h"
#include "util.h"

void *poller_run(void *ptr)
{
        mt_context *thread_context = (mt_context *) ptr;
        poller_ctx *poller_context = (poller_ctx *) thread_context->param;

        unsigned id = thread_context->thread_id;
        rtgtargets *targets = poller_context->targets;

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
                queryhost *host;
                while (!thread_stop_requested && (host = rtgtargets_next(targets))) {
                        cllog(2, "Thread %d picked host '%s'.", id, host->host);
                        // Process the host and get back a list of SQL updates to execute.
                        char **host_queries = get_inserts(host);
                        unsigned n_queries;
                        for (n_queries = 0; host_queries[n_queries]; n_queries++);

                        if (n_queries > 0) {
                                cllog(2, "Thread %u queueing %u queries.", id, n_queries);

                                unsigned  i;
                                for (i = 0; i < n_queries && cbuffer_free(queries) > 0; i++) {
                                        void *result = cbuffer_push(queries, strdup(host_queries[i]));
                                        if (!result)
                                                break;
                                }
                                unsigned qd = cbuffer_count(queries);
                                query_queue_depth = query_queue_depth > qd ? query_queue_depth : qd;
                                if (i != n_queries)
                                        cllog(0, "Thread %d dropped queries due to database queue full.", id);
                        }
                        for (n_queries = 0; host_queries[n_queries]; n_queries++)
                                free(host_queries[n_queries]);
                        free(host_queries);
                }

                // Note how long it took.
                end = time(NULL);

                // Prepare for next iteration.
                iterations++;
        }
        return NULL;
}

