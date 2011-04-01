#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mysql.h>
#include "rtgconf.h"
#include "multithread.h"
#include "util.h"
#include "database.h"
#include "globals.h"

#define COMMIT_INTERVAL 100

unsigned long queries_size();
char *dequeue_query();
MYSQL *connection(rtgconf *config);

void *database_run(void *ptr)
{
        mt_context *thread_context = (mt_context *) ptr;
        database_ctx *database_context = (database_ctx *) thread_context->param;
        unsigned my_id = thread_context->thread_id;
        rtgconf *config = database_context->config;

        unsigned useless_iterations = 0;
        unsigned query_counter = 0;

        MYSQL *conn = 0;

        while (1) {
		if (use_db) {
                if (queries_size() > 0) {
                        if (conn == 0 || mysql_ping(conn) != 0) {
                                if (conn)
                                        mysql_close(conn);
                                cllog(2, "DB thread %d connecting to MySQL.", my_id);
                                conn = connection(config);
                        } else {
                                useless_iterations = 0;
                                query_counter++;

                                if (query_counter % COMMIT_INTERVAL == 0) {
                                        mysql_commit(conn);
                                        cllog(2, "DB thread %d committed transaction due to configured commit interval (query_counter = %u).", my_id, query_counter);
                                }

                                char *query = dequeue_query();
                                mysql_query(conn, query);
                                free(query);
                        }
                } else {
                        useless_iterations++;

                        if (useless_iterations > 5) {
                                mysql_commit(conn);
                                cllog(2, "DB thread %d committed transaction at end of run (query_counter = %u).", my_id, query_counter);
                                query_counter = 0;
                        }

                        if (useless_iterations > 15 && conn != 0 && mysql_ping(conn)) {
                                cllog(2, "DB thread %d disconnecting.", my_id);
                                mysql_close(conn);
                                conn = 0;
                        }

                        sleep(1);
                }
		} else {
                                char *query = dequeue_query();
				printf("%s", query);
		}
        }
        return 0;
}

unsigned long queries_size()
{
        pthread_mutex_lock(&db_list_lock);
        unsigned qs = cbuffer_count(queries);
        pthread_mutex_unlock(&db_list_lock);
        return qs;
}

char *dequeue_query()
{
        pthread_mutex_lock(&db_list_lock);
        char *query =  (char *) cbuffer_pop(queries);
        pthread_mutex_unlock(&db_list_lock);
        return query;
}

MYSQL *connection(rtgconf *config)
{
        MYSQL *conn = mysql_init(NULL);
        if (conn == NULL) {
                cllog(0, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
                return NULL;
        }
        if (mysql_real_connect(conn, config->dbhost, config->dbuser, config->dbpass, config->database, 0, NULL, 0) == NULL) {
                cllog(0, "Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
                return NULL;
        }
        mysql_autocommit(conn, 0);

        return conn;
}
