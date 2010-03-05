#ifndef _GLOBALS_H
#define _GLOBALS_H

#define CLPOLL_VERSION "1.0.3"

#include <pthread.h>
#include <string>
#include <queue>

#include "types.h"
#include "rtgconf.h"
#include "rtgtargets.h"

// See globals.cpp for documentation on what these are used for.

extern RTGTargets hosts;
extern RTGConf config;
extern std::vector<ResultCache> cache;
extern std::queue<std::string> queries;
extern unsigned query_queue_depth;
extern unsigned thread_id;
extern std::string rtgconf;
extern std::string targets;
extern int verbosity;
extern int detach;
extern int use_db;
extern int allow_db_zero;
extern unsigned max_queue_length;
extern pthread_mutex_t global_lock;
extern pthread_mutex_t db_list_lock;
extern pthread_mutex_t cerr_lock;
extern pthread_cond_t global_cond;
extern unsigned active_threads;
extern unsigned stat_inserts;
extern unsigned stat_queries;
extern unsigned stat_iterations;

#endif
