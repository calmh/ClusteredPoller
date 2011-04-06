#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <pthread.h>

// See globals.c for documentation on what these are used for.

#define MIN_QUEUE_LENGTH 100
#define DEFAULT_QUEUE_LENGTH 10000
#define DEFAULT_DBTHREADS_DIVISOR 8

struct clbuf;

extern struct clbuf *queries;
extern unsigned long query_queue_depth;
extern int full_stop_requested;
extern int thread_stop_requested;
extern char *rtgconf_file;
extern char *targets_file;
extern int verbosity;
extern int detach;
extern int use_db;
extern int use_rate_column;
extern int allow_db_zero;
extern int dbthreads_divisor;
extern unsigned max_queue_length;
extern pthread_mutex_t global_lock;
extern pthread_mutex_t cerr_lock;
extern pthread_cond_t global_cond;
extern unsigned active_threads;
extern unsigned stat_inserts;
extern unsigned stat_queries;
extern unsigned stat_iterations;
extern unsigned stat_snmp_fail;
extern unsigned stat_snmp_success;
extern unsigned stat_dropped_queries;

#endif
