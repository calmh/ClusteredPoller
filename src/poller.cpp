#include "globals.h"
#include "poller.h"
#include "snmp.h"
#include "queryablehost.h"

#include <iostream>
#define MAXERRORSPERHOST 3

using namespace std;

Poller::Poller(int num_threads) : Multithread(num_threads)
{
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
        // Assign our offset and stride values.
        unsigned stride = config.threads;
        pthread_mutex_lock(&global_lock);
        unsigned offset = *((int*) id_ptr);
        pthread_mutex_unlock(&global_lock);

        // Start looping.
        unsigned iterations = 0;
        time_t start = 0, end = 0;
        while (1) {

                // Mark ourself sleeping
                if (iterations > 0 && verbosity >= 2) {
                        pthread_mutex_lock(&cerr_lock);
                        cerr << "Thread " << offset << " sleeping after " << end - start << " s processing time." << endl;
                        pthread_mutex_unlock(&cerr_lock);
                }

                // Wait for green light.
                pthread_mutex_lock(&global_lock);
                if (iterations > 0)
                        active_threads--;
                pthread_cond_wait(&global_cond, &global_lock);
                pthread_mutex_unlock(&global_lock);

                if (verbosity >= 2) {
                        pthread_mutex_lock(&cerr_lock);
                        cerr << "Thread " << offset << " starting." << endl;
                        pthread_mutex_unlock(&cerr_lock);
                }
                // Mark ourself active.
                pthread_mutex_lock(&global_lock);
                active_threads++;
                pthread_mutex_unlock(&global_lock);

                // Note our start time, so we know how long an iteration takes.
                start = time(NULL);
                // Loop over our share of the hosts.
                for (unsigned i = offset; i < hosts.size(); i += stride) {
                        QueryHost host = hosts[i];
                        if (verbosity >= 2) {
                                pthread_mutex_lock(&cerr_lock);
                                cerr << "Thread " << offset << " picked host #" << i << ": " << host.host << "." << endl;
                                pthread_mutex_unlock(&cerr_lock);
                        }
                        // Process the host and get back a list of SQL updates to execute.
                        QueryableHost queryable_host(host, cache[i]);
                        vector<string> host_queries = queryable_host.get_inserts();

                        if (host_queries.size() > 0) {
                                if (verbosity >= 2) {
                                        pthread_mutex_lock(&cerr_lock);
                                        cerr << "Thread " << offset << " queueing " << host_queries.size() << " queries." << endl;
                                        pthread_mutex_unlock(&cerr_lock);
                                }

                                vector<string>::iterator it;
                                pthread_mutex_lock(&db_list_lock);
                                for (it = host_queries.begin(); it != host_queries.end() && queries.size() < max_queue_length; it++) {
                                        queries.push(*it);
                                }
                                unsigned qd = queries.size();
                                query_queue_depth = query_queue_depth > qd ? query_queue_depth : qd;
                                if (it != host_queries.end() && verbosity > 0) {
                                        pthread_mutex_lock(&cerr_lock);
                                        cerr << "Thread " << offset << " dropped queries due to database queue full." << endl;
                                        pthread_mutex_unlock(&cerr_lock);
                                }
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

