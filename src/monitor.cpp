#include <iostream>

#include "util.h"
#include "types.h"
#include "monitor.h"
#include "queryablehost.h"
#include "globals.h"

using namespace std;

Monitor::Monitor() : Multithread(1)
{
}

void Monitor::create_thread(pthread_t* thread, int* thread_id)
{
        pthread_create(thread, NULL, &Monitor::run, (void*)thread_id);
}

void* Monitor::run(void* id_ptr)
{
        //int thread_id = *((int*)id_ptr);
        time_t interval = 0; //(time(NULL) / config.interval + 1) * config.interval;
        time_t iteration_completed = 0;
        time_t in_iteration = 0;
        while (1) {
                sleep(1);
                if (active_threads == 0 && in_iteration) {
                        stat_iterations++;
                        if (verbosity > 0) {
                                pthread_mutex_lock(&db_list_lock);
                                unsigned qd = queries.size();
                                pthread_mutex_unlock(&db_list_lock);
                                log(1, "Iteration complete, elapsed time for #%d was %d s.", stat_iterations, time(NULL) - in_iteration);
                                log(1, "Time until next iteration is %d s.", interval - time(NULL));
                                log(1, "  Rows inserted: %d", stat_inserts);
                                log(1, "  Queries queued: %d", stat_queries);
                                log(1, "  Max queue depth: %d", query_queue_depth);
                                log(1, "  Remaining queue: %d", qd);
                                if (qd > 0)
                                        iteration_completed = time(NULL);
                        }
                        stat_inserts = 0;
                        stat_queries = 0;
                        in_iteration = 0;
                        query_queue_depth = 0;
                }

                if (active_threads == 0 && iteration_completed) {
                        pthread_mutex_lock(&db_list_lock);
                        unsigned qd = queries.size();
                        pthread_mutex_unlock(&db_list_lock);
                        if (qd == 0) {
                                log(1,  "  Queue empty %d s after poll completion.", time(NULL) - iteration_completed);
                                iteration_completed = 0;
                        }
                }

                pthread_mutex_lock(&global_lock);
                if (active_threads == 0 && time(NULL) > interval) {
                        interval = (time(NULL) / config.interval + 1) * config.interval;
                        in_iteration = time(NULL);
                        if (verbosity >= 1) {
                                log(1, "Monitor signals wakeup.");
                                pthread_mutex_lock(&db_list_lock);
                                unsigned qd = queries.size();
                                pthread_mutex_unlock(&db_list_lock);
                                if (qd > 0 && verbosity > 0)
                                        log(1, "  Queue at depth %d at poll start.", qd);
                                iteration_completed = 0;
                        }
                        pthread_cond_broadcast(&global_cond);
                }
                pthread_mutex_unlock(&global_lock);
        }
        return NULL;
}
