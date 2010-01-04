#include "types.h"

// Global variables are instantiated here.

using namespace std;

// All hosts that we are to poll.
vector<QueryHost> hosts;
// Cache of latest values and times for rate calculation.
vector<ResultCache> cache;
// Configuration data (database information etc.)
RTGConf config;
// Used for assigning thread ID:s at startup.
unsigned thread_id = 0;
// Queue of outstanding database queries.
list<string> queries;
// Maximum number of outstanding database queries.
unsigned query_queue_depth = 0;

// Configuration variables that are modified by command line flags.
string rtgconf = "/usr/local/rtg/etc/rtg.conf";
string targets = "/usr/local/rtg/etc/targets.cfg";
int verbosity = 0;
int detach = 1;
int use_db = 1;
int allow_db_zero = 0;
unsigned max_queue_length = 10000;

// Locking and statistics
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t db_list_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cerr_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t global_cond = PTHREAD_COND_INITIALIZER;
unsigned active_threads = 0;
unsigned stat_inserts = 0;
unsigned stat_queries = 0;
unsigned stat_iterations = 0;

