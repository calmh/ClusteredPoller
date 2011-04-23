//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

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
#include "clgstr.h"
#include "clinsert.h"

#define COMMIT_INTERVAL 100

unsigned process_database_queue(MYSQL *conn, struct rtgconf *config);
unsigned print_database_queue(struct rtgconf *config);
MYSQL *connection(struct rtgconf *config);
char *build_insert_query(struct clinsert *insert, struct rtgconf *config);

void *database_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct database_ctx *database_context = (struct database_ctx *) thread_context->param;
        unsigned my_id = thread_context->thread_id;
        struct rtgconf *config = database_context->config;

        MYSQL *conn = 0;

        while (!thread_stop_requested) {
                struct timeval start_ts;
                gettimeofday(&start_ts, NULL);
                unsigned query_counter = 0;

                if (!config->use_db) {
                        query_counter = print_database_queue(config);
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

                        query_counter = process_database_queue(conn, config);
                }

                if (query_counter > 0) {
                        struct timeval end_ts;
                        gettimeofday(&end_ts, NULL);
                        unsigned runtime_ms = (end_ts.tv_sec - start_ts.tv_sec) * 1000 + (end_ts.tv_usec - start_ts.tv_usec) / 1000;
                        double tps = 1000.0 * query_counter / (double) runtime_ms;
                        cllog(2, " %6u queries in %6u ms (%.f queries/s) executed on DB by thread %d", query_counter, runtime_ms, tps, my_id);
                }
                sleep(1);
        }

        if (conn) {
                cllog(1, "DB thread %d shutting down.", my_id);
                mysql_commit(conn);
                mysql_close(conn);
        }

        return 0;
}

unsigned process_database_queue(MYSQL *conn, struct rtgconf *config)
{
        unsigned query_counter = 0;

        while (clbuf_count_used(queries) > 0) {
                struct clinsert *insert = (struct clinsert *) clbuf_pop(queries);
                if (insert) {
                        char *query = build_insert_query(insert, config);
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
                        clinsert_free(insert);
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

        return query_counter;
}

unsigned print_database_queue(struct rtgconf *config)
{
        unsigned counter = 0;
        while (clbuf_count_used(queries) > 0) {
                struct clinsert *insert = (struct clinsert *) clbuf_pop(queries);
                if (insert) {
                        char *query = build_insert_query(insert, config);
                        if (query) {
                                cllog(3, "%s", query);
                                free(query);
                                counter++;
                        }
                        clinsert_free(insert);
                }
        }

        return counter;
}

MYSQL *connection(struct rtgconf *config)
{
        MYSQL *conn = mysql_init(NULL);

        if (conn == NULL) {
                cllog(0, "MySQL error %u: %s", mysql_errno(conn), mysql_error(conn));
                return NULL;
        }

        if (mysql_real_connect(conn, config->dbhost, config->dbuser, config->dbpass, config->database, 0, NULL, 0l) == NULL) {
                cllog(0, "MySQL error %u: %s", mysql_errno(conn), mysql_error(conn));
                return NULL;
        }

        my_bool reconnect = 1;
        mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
        mysql_autocommit(conn, (my_bool) 0);

        return conn;
}

char *build_insert_query(struct clinsert *insert, struct rtgconf *config)
{
        struct clgstr *gs = clgstr_create(64);
        clgstr_append(gs, "INSERT INTO ");
        clgstr_append(gs, insert->table);
        if (config->use_rate_column)
                clgstr_append(gs, " (id, dtime, counter, rate) VALUES ");
        else
                clgstr_append(gs, " (id, dtime, counter) VALUES ");

        int rows = 0;
        const int buffer_length = 32;
        char buffer[buffer_length];
        unsigned i;
        for (i = 0; i < insert->nvalues; i++) {
                if (config->allow_db_zero || insert->values[i].rate) {
                        if (rows > 0)
                                clgstr_append(gs, ", ");

                        // Begin series
                        clgstr_append(gs, "(");

                        // ID
                        snprintf(buffer, buffer_length, "%u", insert->values[i].id);
                        clgstr_append(gs, buffer);

                        // Time stamp, in UTC
                        clgstr_append(gs, ", FROM_UNIXTIME(");
                        snprintf(buffer, buffer_length, "%lu", insert->values[i].dtime);
                        clgstr_append(gs, buffer);
                        clgstr_append(gs, ")");

                        // Counter
                        clgstr_append(gs, ", ");
                        snprintf(buffer, buffer_length, "%llu", insert->values[i].counter);
                        clgstr_append(gs, buffer);

                        if (config->use_rate_column) {
                                // Rate
                                clgstr_append(gs, ", ");
                                snprintf(buffer, buffer_length, "%u", insert->values[i].rate);
                                clgstr_append(gs, buffer);
                        }
                        // End series
                        clgstr_append(gs, ")");
                        rows++;
                }
        }
        if (rows > 0) {
                char *return_str = strdup(clgstr_string(gs));
                clgstr_free(gs);
                return return_str;
        } else {
                clgstr_free(gs);
                return NULL;
        }
}
