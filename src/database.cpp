#include "database.h"
#include "globals.h"

#ifdef USE_MYSQL
#include <mysql++.h>
#endif

#include <iostream>
using namespace std;

Database::Database(int num_threads) : Multithread(num_threads)
{
}

void Database::create_thread(pthread_t* thread, int thread_id)
{
        pthread_create(thread, NULL, &Database::run, (void*)&thread_id);
}

void* Database::run(void *id_ptr)
{
        pthread_mutex_lock(&global_lock);
        pthread_mutex_unlock(&global_lock);

#ifdef USE_MYSQL
        unsigned my_id = thread_id;
        mysqlpp::Connection conn(false);
        int useless_iterations = 0;
#endif
        while (1) {
                pthread_mutex_lock(&db_list_lock);
                unsigned qs = queries.size();
                pthread_mutex_unlock(&db_list_lock);
                if (qs > 0) {
#ifdef USE_MYSQL
                        if (!conn.connected()) {
                                if (verbosity >= 2) {
                                        pthread_mutex_lock(&cerr_lock);
                                        cerr << "DB thread " << my_id << " connecting to MySQL" << endl;
                                        pthread_mutex_unlock(&cerr_lock);
                                }
                                conn.connect(config.database.c_str(), config.dbhost.c_str(), config.dbuser.c_str(), config.dbpass.c_str());
                                if (!conn.connected()) {
                                        pthread_mutex_lock(&cerr_lock);
                                        cerr << "DB thread " << my_id << " !! Connection failed." << endl;
                                        pthread_mutex_unlock(&cerr_lock);
                                }
                        } else {
                                useless_iterations = 0;
                                mysqlpp::Query q = conn.query(dequeue_query());
                                q.exec();
                        }
#else
                        cerr << dequeue_query() << endl;
#endif
                } else {
#ifdef USE_MYSQL
                        useless_iterations++;
                        if (useless_iterations > 10 && conn.connected()) {
                                if (verbosity >= 2) {
                                        pthread_mutex_lock(&cerr_lock);
                                        cerr << "DB thread " << my_id << " disconnecting from MySQL" << endl;
                                        pthread_mutex_unlock(&cerr_lock);
                                }
                                conn.disconnect();
                        }
#endif
                        sleep(1);
                }
        }
}

// Exctract a query from the queue, or an empty string if it's empty.
string Database::dequeue_query()
{
        pthread_mutex_lock(&db_list_lock);
        unsigned qs = queries.size();
        if (qs == 0)
                return "";
        string q = queries.front();
        queries.pop_front();
        pthread_mutex_unlock(&db_list_lock);
        return q;
}

