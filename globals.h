#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "types.h"

// Global variables.
extern std::vector<QueryHost> hosts;
extern std::vector<ResultCache> cache;
extern RTGConf config;
extern unsigned thread_id;

// Command line flags
extern std::string rtgconf;
extern std::string targets;
extern int verbosity;
extern int detach;
extern int use_db;
extern int allow_db_zero;

// Locking and statistics
extern pthread_mutex_t global_lock;
extern unsigned active_threads;
extern unsigned stat_inserts;
extern unsigned stat_queries;
extern unsigned stat_iterations;

#endif
