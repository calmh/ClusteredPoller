#include "types.h"
#include "rtgconf.h"
#include "rtgtargets.h"

#include <queue>
#include <string>
using namespace std;

// Global variables are instantiated here.

// All hosts that we are to poll.
RTGTargets hosts;
// Configuration data (database information etc.)
RTGConf config;
// Cache of latest values and times for rate calculation.
vector<ResultCache> cache;
// Used for assigning thread ID:s at startup.
unsigned thread_id = 0;
// Queue of outstanding database queries.
queue<string> queries;
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

