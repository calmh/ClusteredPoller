#include <iostream>

#include "util.h"
#include "types.h"
#include "globals.h"
#include "poller.h"
#include "snmp.h"
#include "queryablehost.h"

using namespace std;

int Poller::stride;
rtgtargets* Poller::hosts;
vector<ResultCache> *Poller::cache;

Poller::Poller(int num_threads, rtgtargets* hosts) : Multithread(num_threads)
{
        stride = num_threads;
        Poller::hosts = hosts;
        Poller::cache = new vector<ResultCache>(hosts->nhosts);
}

void Poller::create_thread(pthread_t* thread, int* thread_id)
{
        pthread_create(thread, NULL, &Poller::run, (void*)thread_id);
}

// Continuosly loop over all the hosts and do all processing for them.
// Each thread has an offset (= thread id) and stride (= number of threads).
// The list of hosts is processed starting at "offset" and then incrementing
// with "stride" each iteration. This way we avoid having a separate host list
// for each thread, and the threads keep off each others toes.
void* Poller::run(void* id_ptr)
{
        pthread_mutex_lock(&global_lock);
        unsigned offset = *((int*) id_ptr);
        pthread_mutex_unlock(&global_lock);

        // Start looping.
        unsigned iterations = 0;
        time_t start = 0, end = 0;
        while (1) {

                // Mark ourself sleeping
                if (iterations > 0)
                        log(2, "Thread %d sleeping after %d s processing time.", offset, end - start);

                // Wait for green light.
                pthread_mutex_lock(&global_lock);
                if (iterations > 0)
                        active_threads--;
                pthread_cond_wait(&global_cond, &global_lock);
                pthread_mutex_unlock(&global_lock);

                log(2, "Thread %d starting.", offset);
                // Mark ourself active.
                pthread_mutex_lock(&global_lock);
                active_threads++;
                pthread_mutex_unlock(&global_lock);

                // Note our start time, so we know how long an iteration takes.
                start = time(NULL);
                // Loop over our share of the hosts.
                for (unsigned i = offset; i < hosts->nhosts; i += stride) {
                        queryhost* host = hosts->hosts[i];
                        log(2, "Thread %d picked host #%d.", offset, i);
                        // Process the host and get back a list of SQL updates to execute.
                        QueryableHost queryable_host(host, (*cache)[i]);
                        vector<string> host_queries = queryable_host.get_inserts();

                        if (host_queries.size() > 0) {
                                log(2, "Thread %d queueing %d queries.", offset, host_queries.size());

                                vector<string>::iterator it;
                                pthread_mutex_lock(&db_list_lock);
                                for (it = host_queries.begin(); it != host_queries.end() && queries.size() < max_queue_length; it++) {
                                        queries.push(*it);
                                }
                                unsigned qd = queries.size();
                                query_queue_depth = query_queue_depth > qd ? query_queue_depth : qd;
                                if (it != host_queries.end())
                                        log(0, "Thread %d dropped queries due to database queue full.", offset);
                                pthread_mutex_unlock(&db_list_lock);

                        }
                }

                // Note how long it took.
                end = time(NULL);

                // Prepare for next iteration.
                iterations++;
        }
        return NULL;
}

