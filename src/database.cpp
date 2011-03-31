#ifdef USE_MYSQL
#include <mysql++.h>
#endif
#include <iostream>


#include "util.h"
#include "types.h"
#include "database.h"
#include "globals.h"

using namespace std;

RTGConf* Database::config;

#define COMMIT_INTERVAL 100

Database::Database(int num_threads, RTGConf* config) : Multithread(num_threads)
{
        Database::config = config;
}

Database::~Database()
{
}

void Database::create_thread(pthread_t* thread, int* thread_id)
{
        pthread_create(thread, NULL, &Database::run, (void*)thread_id);
}

#ifdef USE_MYSQL
void* Database::run(void* id_ptr)
{
        pthread_mutex_lock(&global_lock);
        int my_id = *((int*) id_ptr);
        pthread_mutex_unlock(&global_lock);

        unsigned useless_iterations = 0;
        unsigned query_counter = 0;

        mysqlpp::Connection* conn = 0;
        mysqlpp::Transaction* transaction = 0;

        while (1) {
                if (queries_size() > 0) {
                        if (conn == 0 || !conn->connected()) {
                                conn = connection(my_id);
                        } else {
                                useless_iterations = 0;
                                query_counter++;

                                if (transaction != 0 && query_counter % COMMIT_INTERVAL == 0) {
                                        transaction->commit();
                                        delete transaction;
                                        transaction = 0;
                                        log(2, "DB thread %d committed transaction due to configured commit interval (query_counter = %u).", my_id, query_counter);
                                }

                                if (transaction == 0) {
                                        transaction = new mysqlpp::Transaction(*conn, false);
                                        log(2, "DB thread %d started transaction.", my_id);
                                }

                                mysqlpp::Query q = conn->query(dequeue_query());
                                q.exec();
                        }
                } else {
                        useless_iterations++;

                        if (useless_iterations > 5 && transaction != 0) {
                                transaction->commit();
                                delete transaction;
                                transaction = 0;
                                log(2, "DB thread %d committed transaction at end of run (query_counter = %u).", my_id, query_counter);
                                query_counter = 0;
                        }

                        if (useless_iterations > 15 && conn != 0 && conn->connected()) {
                                log(2, "DB thread %d disconnecting.", my_id);
                                conn->disconnect();
                                delete conn;
                                conn = 0;
                        }

                        sleep(1);
                }
        }
        return 0;
}
#else
void* Database::run(void* id_ptr)
{
        while (1) {
                pthread_mutex_lock(&db_list_lock);
                unsigned qs = queries.size();
                pthread_mutex_unlock(&db_list_lock);
                if (qs > 0) {
                        cerr << dequeue_query() << endl;
                } else {
                        sleep(1);
                }
        }
        return 0;
}
#endif

// Exctract a query from the queue, or an empty string if it's empty.
string Database::dequeue_query()
{
        pthread_mutex_lock(&db_list_lock);
        unsigned qs = queries.size();
        if (qs == 0) {
                pthread_mutex_unlock(&db_list_lock);
                return "";
        }

        string q = queries.front();
        queries.pop();
        pthread_mutex_unlock(&db_list_lock);
        return q;
}

unsigned Database::queries_size()
{
        pthread_mutex_lock(&db_list_lock);
        unsigned qs = queries.size();
        pthread_mutex_unlock(&db_list_lock);
        return qs;
}

mysqlpp::Connection* Database::connection(int my_id)
{
        log(2, "DB thread %d connecting to MySQL.", my_id);

        mysqlpp::Connection* conn = new mysqlpp::Connection(false);
        conn->connect(config->database.c_str(), config->dbhost.c_str(), config->dbuser.c_str(), config->dbpass.c_str());

        if (conn->connected()) {
                mysqlpp::Query disable_autocommit = conn->query("SET AUTOCOMMIT=0");
                disable_autocommit.exec();
        } else {
                log(0, "DB thread %d connection failed.", my_id);
        }

        return conn;
}
