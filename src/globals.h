#ifndef GLOBALS_H
#define GLOBALS_H

/// @file globals.h
/// Global variables.

#include <pthread.h>
#include <sys/time.h>

/// Minimum allowable database queue length. The user cannot select a smaller queue than this.
#define MIN_QUEUE_LENGTH 100
/// Default database queue length.
#define DEFAULT_QUEUE_LENGTH 10000
/// Default divisor to use to calculate number of database threads:
/// num_db_threads = num_poller_threads / dbthreads_divisor
#define DEFAULT_DBTHREADS_DIVISOR 8
/// Default path to RTG configuration file.
#define DEFAULT_RTGCONF_FILE "/usr/local/rtg/etc/rtg.conf"
/// Default path to RTG targets file.
#define DEFAULT_TARGETS_FILE "/usr/local/rtg/etc/targets.cfg"

struct clbuf;

/// Object to hold statistics per polling interval.
struct statistics {
        unsigned insert_rows; ///< Number of rows inserted.
        unsigned insert_queries; ///< Number of queries queued.
        unsigned dropped_queries; ///< Number of queries that couldn't be queued due to lack of buffer space.
        unsigned iterations; ///< Number of polling iterations completed. Increases by one every polling interval.
        unsigned snmp_fail; ///< Number of SNMP queries that failed.
        unsigned snmp_success; ///< Number of SNMP queries that were successfull.
        unsigned max_queue_depth;  ///< Maximum database queue depth seen during last polling interval.
        struct timeval query_threads_finished; ///< Timestamp when all poller threads were complete.
};

/// Queue of outstanding database queries.
extern struct clbuf *queries;

/// User has requested we stop all threads and exit (SIGTERM).
extern int full_stop_requested;
/// User has requested we stop all threads and reload (SIGHUP).
extern int thread_stop_requested;

/// Global verbosity level (default 0).
extern int verbosity;

/// Number of currently active poller threads.
extern unsigned active_threads;
/// The statistics object.
extern struct statistics statistics;
/// Global lock to be acquired before manipulating statistics, and used for thread synchronization.
extern pthread_mutex_t global_lock;
/// Global lock used for not mangling stderr output.
extern pthread_mutex_t cerr_lock;
/// Conditional used to broadcast "go!" to poller threads.
extern pthread_cond_t global_cond;

#endif                          /* GLOBALS_H */
