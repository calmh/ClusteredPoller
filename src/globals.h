#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <sys/time.h>

// See globals.c for documentation on what these are used for.

#define MIN_QUEUE_LENGTH 100
#define DEFAULT_QUEUE_LENGTH 10000
#define DEFAULT_DBTHREADS_DIVISOR 8
#define DEFAULT_RTGCONF_FILE "/usr/local/rtg/etc/rtg.conf"
#define DEFAULT_TARGETS_FILE "/usr/local/rtg/etc/targets.cfg"

struct clbuf;

struct statistics {
        unsigned insert_rows;
        unsigned insert_queries;
        unsigned dropped_queries;
        unsigned iterations;
        unsigned snmp_fail;
        unsigned snmp_success;
        unsigned max_queue_depth;
        struct timeval query_threads_finished;
};

// Queue of outstanding database queries.
extern struct clbuf *queries;

// Signal flags
extern int full_stop_requested;
extern int thread_stop_requested;

// Configuration variables that are modified by command line flags.
extern int verbosity;

// Locking and statistics
extern unsigned active_threads;
extern struct statistics statistics;
extern pthread_mutex_t global_lock;
extern pthread_mutex_t cerr_lock;
extern pthread_cond_t global_cond;

#endif                          /* GLOBALS_H */
