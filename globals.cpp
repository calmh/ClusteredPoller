#include "types.h"

using namespace std;

// Global variables.
vector<QueryHost> hosts;
vector<ResultCache> cache;
RTGConf config;
unsigned thread_id = 0;

// Command line flags
string rtgconf = "/usr/local/rtg/etc/rtg.conf";
string targets = "/usr/local/rtg/etc/targets.cfg";
int verbosity = 0;
int detach = 1;
int use_db = 1;
int allow_db_zero = 0;

// Locking and statistics
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
unsigned active_threads = 0;
unsigned stat_inserts = 0;
unsigned stat_queries = 0;
unsigned stat_iterations = 0;

