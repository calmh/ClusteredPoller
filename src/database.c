#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mysql.h>
#include <string.h>

#include "clbuf.h"
#include "rtgconf.h"
#include "multithread.h"
#include "cllog.h"
#include "database.h"
#include "globals.h"
#include "queryablehost.h"
#include "clgstr.h"

#define COMMIT_INTERVAL 100

void process_database_queue(MYSQL *conn);
void print_database_queue();
MYSQL *connection(struct rtgconf *config);
char *build_insert_query(struct db_insert *insert);

void *database_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct database_ctx *database_context = (struct database_ctx *) thread_context->param;
        unsigned my_id = thread_context->thread_id;
        struct rtgconf *config = database_context->config;

        MYSQL *conn = 0;

        while (!thread_stop_requested) {
                if (!use_db) {
                        print_database_queue();
                } else {
                        if (!conn) {
                                cllog(1, "DB thread %d connecting to MySQL.", my_id);
                                conn = connection(config);
                                if (!conn) {
                                        sleep(5);
                                        continue;
                                }
                        }
                        if (mysql_ping(conn) != 0) {
                                cllog(0, "MySQL error %u: %s", mysql_errno(conn), mysql_error(conn));
                                sleep(5);
                                continue;
                        }

                        process_database_queue(conn);
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

void process_database_queue(MYSQL *conn)
{
        unsigned query_counter = 0;

        while (clbuf_count_used(queries) > 0) {
                struct db_insert *insert = (struct db_insert *) clbuf_pop(queries);
                if (insert) {
                        char *query = build_insert_query(insert);
                        if (query) {
                                int result = mysql_query(conn, query);
                                if (result == 0)
                                        query_counter++;
                                else
                                        cllog(0, "MySQL error %u: %s", mysql_errno(conn), mysql_error(conn));
                                free(query);
                        } else {
                                cllog(4, "DB thread dequeued NULL query.");
                        }
                        db_insert_free(insert);
                }

                if (query_counter > 0 && query_counter % COMMIT_INTERVAL == 0) {
                        mysql_commit(conn);
                        cllog(2, "DB thread committed transaction due to configured commit interval (query_counter = %u).", query_counter);
                }
        }

        if (query_counter > 0) {
                mysql_commit(conn);
                cllog(2, "DB thread committed transaction at end of run (query_counter = %u).", query_counter);
        }
}

void print_database_queue()
{
        while (clbuf_count_used(queries) > 0) {
                struct db_insert *insert = (struct db_insert *) clbuf_pop(queries);
                if (insert) {
                        char *query = build_insert_query(insert);
                        if (query) {
                                cllog(3, "%s", query);
                                free(query);
                        }
                        db_insert_free(insert);
                }
        }
}

MYSQL *connection(struct rtgconf *config)
{
        MYSQL *conn = mysql_init(NULL);

        if (conn == NULL) {
                cllog(0, "MySQL error %u: %s", mysql_errno(conn), mysql_error(conn));
                return NULL;
        }

        if (mysql_real_connect(conn, config->dbhost, config->dbuser, config->dbpass, config->database, 0, NULL, 0) == NULL) {
                cllog(0, "MySQL error %u: %s", mysql_errno(conn), mysql_error(conn));
                return NULL;
        }

        my_bool reconnect = 1;
        mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
        mysql_autocommit(conn, 0);

        return conn;
}

char *build_insert_query(struct db_insert *insert)
{
        struct clgstr *gs = clgstr_create(64);
        clgstr_append(gs, "INSERT INTO ");
        clgstr_append(gs, insert->table);
        if (use_rate_column)
                clgstr_append(gs, " (id, dtime, counter, rate) VALUES ");
        else
                clgstr_append(gs, " (id, dtime, counter) VALUES ");

        int rows = 0;
        char buffer[16];
        unsigned i;
        for (i = 0; i < insert->nvalues; i++) {
                if (allow_db_zero || insert->values[i].rate) {
                        if (rows > 0)
                                clgstr_append(gs, ", ");
                        clgstr_append(gs, "(");
                        snprintf(buffer, 15, "%u", insert->values[i].id);
                        clgstr_append(gs, buffer);
                        clgstr_append(gs, ", FROM_UNIXTIME(");
                        snprintf(buffer, 15, "%lu", insert->values[i].dtime);
                        clgstr_append(gs, buffer);
                        clgstr_append(gs, "), ");
                        snprintf(buffer, 15, "%llu", insert->values[i].counter);
                        clgstr_append(gs, buffer);
                        if (use_rate_column) {
                                clgstr_append(gs, ", ");
                                snprintf(buffer, 15, "%u", insert->values[i].rate);
                                clgstr_append(gs, buffer);
                        }
                        clgstr_append(gs, ")");
                        rows++;
                }
        }
        if (rows > 0) {
                char *return_str = strdup(gs->string);
                clgstr_free(gs);
                return return_str;
        } else {
                clgstr_free(gs);
                return NULL;
        }
}
