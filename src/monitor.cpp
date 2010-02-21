#include "monitor.h"
#include "queryablehost.h"
#include "globals.h"

#include <iostream>
using namespace std;

Monitor::Monitor() : Multithread(1)
{
}

void Monitor::create_thread(pthread_t* thread, int thread_id)
{
    pthread_create(thread, NULL, &Monitor::run, (void*)&thread_id);
}

void* Monitor::run(void *id_ptr)
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
				pthread_mutex_lock(&cerr_lock);
				cerr << "Iteration complete, elapsed time for #" << stat_iterations << " was " << time(NULL) - in_iteration << " s." << endl;
				cerr << "Time until next iteration is " << interval - time(NULL) << " s." << endl;
				cerr << "  Rows inserted: " << stat_inserts << endl;
				cerr << "  Queries queued: " << stat_queries << endl;
				cerr << "  Max queue depth: " << query_queue_depth << endl;
				cerr << "  Remaining queue: " << qd << endl;
				pthread_mutex_unlock(&cerr_lock);
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
				if (verbosity > 0)
					cerr << "  Queue zero " << time(NULL) - iteration_completed << " s after poll completion." << endl;
				iteration_completed = 0;
			}
		}

		pthread_mutex_lock(&global_lock);
		if (active_threads == 0 && time(NULL) > interval) {
			interval = (time(NULL) / config.interval + 1) * config.interval;
			in_iteration = time(NULL);
			if (verbosity >= 1) {
				pthread_mutex_lock(&cerr_lock);
				cerr << "Monitor signals wakeup." << endl;
				pthread_mutex_unlock(&cerr_lock);
				pthread_mutex_lock(&db_list_lock);
				unsigned qd = queries.size();
				pthread_mutex_unlock(&db_list_lock);
				if (qd > 0 && verbosity > 0)
					cerr << "  Queue at depth " << qd << " at poll start." << endl;
				iteration_completed = 0;
			}
			pthread_cond_broadcast(&global_cond);
		}
		pthread_mutex_unlock(&global_lock);
	}
	return NULL;
}
