#include "globals.h"
#include "clbuf.h"
#include "pthread.h"

struct clbuf *queries;
int full_stop_requested = 0;
int thread_stop_requested = 0;
int verbosity = 0;
unsigned active_threads = 0;
struct statistics statistics = { 0 };

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cerr_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t global_cond = PTHREAD_COND_INITIALIZER;
