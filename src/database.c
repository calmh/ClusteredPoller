#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mysql.h>

#include "clbuf.h"
#include "rtgconf.h"
#include "multithread.h"
#include "util.h"
#include "database.h"
#include "globals.h"

#define COMMIT_INTERVAL 100

MYSQL *connection(struct rtgconf *config);

void *database_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct database_ctx *database_context = (struct database_ctx *) thread_context->param;
        unsigned my_id = thread_context->thread_id;
        struct rtgconf *config = database_context->config;

        unsigned useless_iterations = 0;
        unsigned query_counter = 0;

        MYSQL *conn = 0;

        while (!thread_stop_requested) {
                if (use_db) {
                        if (clbuf_count_used(queries) > 0) {
                                if (conn == 0 || mysql_ping(conn) != 0) {
                                        if (conn)
                                                mysql_close(conn);
                                        cllog(2, "DB thread %d connecting to MySQL.", my_id);
                                        conn = connection(config);
                                        if (conn)
                                                cllog(2, "DB thread %d is connected to MySQL.", my_id);
                                } else {
                                        useless_iterations = 0;
                                        query_counter++;

                                        if (query_counter % COMMIT_INTERVAL == 0) {
                                                mysql_commit(conn);
                                                cllog(2, "DB thread %d committed transaction due to configured commit interval (query_counter = %u).", my_id, query_counter);
                                        }

                                        char *query = (char *) clbuf_pop(queries);
                                        if (query) {
                                                int result = mysql_query(conn, query);
                                                cllog(3, "DB thread %d executed query with result %d.", my_id, result);
                                                free(query);
                                        } else {
                                                cllog(2, "DB thread %d dequeued NULL query.", my_id);
                                        }
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
                        char *query = (char *) clbuf_pop(queries);
                        if (query) {
                                cllog(3, "%s", query);
                                free(query);
                        }
                        if (clbuf_count_used(queries) == 0)
                                sleep(1);
                }
        }

        if (conn) {
                cllog(1, "DB thread %d shutting down.", my_id);
                mysql_commit(conn);
                mysql_close(conn);
        }

        return 0;
}

MYSQL *connection(struct rtgconf *config)
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
