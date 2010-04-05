#include "database.h"
#include "globals.h"

#include <string>
using namespace std;

Database::Database(int num_threads) : Multithread(num_threads)
{
}

void Database::create_thread(pthread_t* thread, int* thread_id)
{
}

void* Database::run(void* id_ptr)
{
        return NULL;
}

string Database::dequeue_query()
{
        return "";
}

