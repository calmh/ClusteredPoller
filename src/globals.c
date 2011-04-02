#include "rtgconf.h"
#include "rtgtargets.h"
#include "cbuffer.h"
#include "pthread.h"

// Global variables are instantiated here.

// Queue of outstanding database queries.
cbuffer *queries;
// Maximum number of outstanding database queries.
unsigned long query_queue_depth = 0;

// Signal flags
int full_stop_requested = 0;
int thread_stop_requested = 0;

// Configuration variables that are modified by command line flags.
char *rtgconf_file = (char *) "/usr/local/rtg/etc/rtg.conf";
char *targets_file = (char *) "/usr/local/rtg/etc/targets.cfg";
int verbosity = 0;
int detach = 1;
int use_db = 1;
int allow_db_zero = 0;
unsigned max_queue_length = 10000;

// Locking and statistics
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cerr_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t global_cond = PTHREAD_COND_INITIALIZER;
unsigned active_threads = 0;
unsigned stat_inserts = 0;
unsigned stat_queries = 0;
unsigned stat_iterations = 0;

