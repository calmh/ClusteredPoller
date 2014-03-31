/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#include "poller.h"

#include "clbuf.h"
#include "clgstr.h"
#include "clinsert.h"
#include "cllog.h"
#include "clsnmp.h"
#include "globals.h"
#include "multithread.h"
#include "rtgtargets.h"
#include "xmalloc.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAXERRORSPERHOST 3

static void thread_inactive(unsigned id, unsigned iterations);
static void thread_active(unsigned id);
static void process_host(unsigned id, struct queryhost *host);
static void process_queries(unsigned id, struct clinsert **host_queries);

void *poller_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct poller_ctx *poller_context = (struct poller_ctx *) thread_context->param;

        unsigned id = thread_context->thread_id;
        struct rtgtargets *targets = poller_context->targets;
        unsigned iterations = 0;

        /* Start looping. */
        while (!thread_stop_requested) {
                struct queryhost *host;

                /* Wait for green light. */
                thread_inactive(id, iterations);

                if (thread_stop_requested)
                        break;

                /* Mark ourself active. */
                thread_active(id);

                /* Loop over our share of the hosts. */
                while (!thread_stop_requested && (host = rtgtargets_next(targets)))
                        process_host(id, host);

                /* Prepare for next iteration. */
                iterations++;
        }
        return NULL;
}

void thread_inactive(unsigned id, unsigned iterations)
{
        pthread_mutex_lock(&global_lock);

        /* Only decrease thread counter if this is not the first iteration. */
        if (iterations > 0) {
                active_threads--;
                cllog(2, "Thread %u blocking.", id);
        }

        if (!active_threads)    /* We are the last one */
                statistics.query_threads_finished = curms();

        /* Wait for monitor to broadcast go signal. */
        pthread_cond_wait(&global_cond, &global_lock);

        pthread_mutex_unlock(&global_lock);
}

void thread_active(unsigned id)
{
        cllog(2, "Thread %u starting.", id);

        pthread_mutex_lock(&global_lock);
        active_threads++;
        pthread_mutex_unlock(&global_lock);
}

void process_host(unsigned id, struct queryhost *host)
{
        struct clinsert **host_queries;

        cllog(2, "Thread %u picked host '%s'.", id, host->host);

        /* Poll the host, get back the list of inserts, and queue them all. */
        host_queries = get_clinserts(host);
        process_queries(id, host_queries);
        free(host_queries);
}

void process_queries(unsigned id, struct clinsert **host_queries)
{
        unsigned n_queries = clinsert_count(host_queries);
        unsigned dropped_queries;
        unsigned queued_queries;
        unsigned queued_values;
        unsigned processed_inserts;
        unsigned depth;

        cllog(2, "Thread %u queueing %u queries.", id, n_queries);

        queued_queries = 0;
        queued_values = 0;
        for (processed_inserts = 0; processed_inserts < n_queries; processed_inserts++) {
                void *result = 0;
                if (clbuf_count_free(queries) > 0)
                        result = clbuf_push(queries, host_queries[processed_inserts]);

                if (result) {
                        queued_queries++;
                        queued_values += host_queries[processed_inserts]->nvalues;
                } else {
                        clinsert_free(host_queries[processed_inserts]);
                }
        }

        depth = clbuf_count_used(queries);
        dropped_queries = n_queries - processed_inserts;

        if (dropped_queries)
                cllog(0, "Thread %u dropped %u queries due to database queue full.", id, dropped_queries);

        /* Update statistics. */
        pthread_mutex_lock(&global_lock);
        statistics.insert_rows += queued_values;
        statistics.insert_queries += queued_queries;
        statistics.dropped_queries += dropped_queries;
        statistics.max_queue_depth = statistics.max_queue_depth > depth ? statistics.max_queue_depth : depth;
        pthread_mutex_unlock(&global_lock);
}

void calculate_rate(time_t prev_time, unsigned long long prev_counter, time_t cur_time, unsigned long long cur_counter, int bits, unsigned long long *counter_diff, unsigned *rate)
{
        if (bits == 0) {
                /* It's a gauge so just return the value as both counter diff and rate. */
                *counter_diff = cur_counter;
                *rate = (unsigned) cur_counter;
        } else {
                time_t time_diff = cur_time - prev_time;
                if (time_diff == 0) {
                        cllog(0, "Fatal error: time_diff == 0 (can't happen!)");
                        exit(-1);
                }
                *counter_diff = cur_counter - prev_counter;
                if (prev_counter > cur_counter) {
                        /* We seem to have a wrap.
                           Wrap it back to find the correct rate. */
                        if (bits == 64)
                                *counter_diff += 18446744073709551615ull + 1;   /* 2^64-1 + 1 */
                        else
                                *counter_diff += 4294967296ull; /* 2^32 */
                }
                /* Return the calculated rate. */
                *rate = (unsigned) (*counter_diff / time_diff);
        }
}

struct clinsert **get_clinserts(struct queryhost *host)
{
        unsigned snmp_fail = 0;
        unsigned snmp_success = 0;

        /* Start a new SNMP session. */
        struct clsnmp_session *session = clsnmp_session_create(host->host, host->community, host->snmpver);

        /* Prepare a list of inserts. */
        struct clinsert **inserts = (struct clinsert **) xmalloc(sizeof(struct clinsert *) * MAX_TABLES);
        memset(inserts, 0, sizeof(struct clinsert *) * MAX_TABLES);

        if (session) {
                int errors = 0;
                /* Iterate over all targets in the host. */
                unsigned i;
                for (i = 0; i < host->nrows; i++) {
                        struct queryrow *row = host->rows[i];

                        unsigned long long counter;
                        time_t dtime;
                        int success = clsnmp_get(session, row->oid, &counter, &dtime);
                        if (success) {
                                if (row->bits == 0 || row->cached_time) {
                                        unsigned long long counter_diff;
                                        unsigned rate;
                                        calculate_rate(row->cached_time, row->cached_counter, dtime, counter, row->bits, &counter_diff, &rate);
                                        if (rate < row->speed) {
                                                struct clinsert *insert = clinsert_for_table(inserts, row->table);
                                                clinsert_push_value(insert, row->id, counter_diff, rate, dtime, counter);
                                        } else {
                                                cllog(1, "Rate %u exceeds speed %llu bps for host %s oid %s", rate, row->speed, host->host, row->oid);
                                        }
                                }
                                row->cached_time = dtime;
                                row->cached_counter = counter;
                                snmp_success++;
                        } else {
                                cllog(1, "SNMP get for %s OID %s failed.", host->host, row->oid);
                                errors++;
                                snmp_fail++;
                        }
                        if (errors >= MAXERRORSPERHOST) {
                                cllog(0, "Too many errors for host %s, aborted.", host->host);
                                break;
                        }
                }
        } else {
                cllog(0, "Error in SNMP setup.");
        }
        clsnmp_session_free(session);

        pthread_mutex_lock(&global_lock);
        statistics.snmp_fail += snmp_fail;
        statistics.snmp_success += snmp_success;
        pthread_mutex_unlock(&global_lock);

        return inserts;
}
