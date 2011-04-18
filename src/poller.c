//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#define _GNU_SOURCE

#include <string.h>
#include <time.h>
#include <stdio.h>

#include "clbuf.h"
#include "clgstr.h"
#include "clinsert.h"
#include "cllog.h"
#include "clsnmp.h"
#include "globals.h"
#include "multithread.h"
#include "poller.h"
#include "rtgtargets.h"
#include "xmalloc.h"

#define MAXERRORSPERHOST 3

void calculate_rate(time_t prev_time, unsigned long long prev_counter, time_t cur_time, unsigned long long cur_counter, int bits, unsigned long long *counter_diff, unsigned *rate);

void *poller_run(void *ptr)
{
        struct mt_context *thread_context = (struct mt_context *) ptr;
        struct poller_ctx *poller_context = (struct poller_ctx *) thread_context->param;

        unsigned id = thread_context->thread_id;
        struct rtgtargets *targets = poller_context->targets;

        // Start looping.
        unsigned iterations = 0;
        time_t start = 0, end = 0;
        while (!thread_stop_requested) {

                // Mark ourself sleeping
                if (iterations > 0)
                        cllog(2, "Thread %d sleeping after %u s processing time.", id, (unsigned) (end - start));

                // Wait for green light.
                pthread_mutex_lock(&global_lock);
                if (iterations > 0)
                        active_threads--;
                if (!active_threads)    // We are the last one
                        gettimeofday(&statistics.query_threads_finished, NULL);
                pthread_cond_wait(&global_cond, &global_lock);
                pthread_mutex_unlock(&global_lock);

                if (thread_stop_requested)
                        break;

                cllog(2, "Thread %d starting.", id);
                // Mark ourself active.
                pthread_mutex_lock(&global_lock);
                active_threads++;
                pthread_mutex_unlock(&global_lock);

                // Note our start time, so we know how long an iteration takes.
                start = time(NULL);
                // Loop over our share of the hosts.
                struct queryhost *host;
                unsigned dropped_queries = 0;
                unsigned queued_queries = 0, queued_values = 0;
                while (!thread_stop_requested && (host = rtgtargets_next(targets))) {
                        cllog(2, "Thread %d picked host '%s'.", id, host->host);
                        // Process the host and get back a list of SQL updates to execute.
                        struct clinsert **host_queries = get_clinserts(host);
                        unsigned n_queries = clinsert_count(host_queries);

                        if (n_queries > 0) {
                                cllog(2, "Thread %u queueing %u queries.", id, n_queries);

                                unsigned i;
                                for (i = 0; i < n_queries && clbuf_count_free(queries) > 0; i++) {
                                        void *result = clbuf_push(queries, host_queries[i]);
                                        if (result) {
                                                queued_queries++;
                                                queued_values += host_queries[i]->nvalues;
                                        } else {
                                                break;
                                        }
                                }
                                unsigned qd = clbuf_count_used(queries);
                                statistics.max_queue_depth = statistics.max_queue_depth > qd ? statistics.max_queue_depth : qd;
                                if (i != n_queries) {
                                        if (!dropped_queries)
                                                cllog(0, "Thread %d dropped queries due to database queue full.", id);
                                        dropped_queries += n_queries - i;
                                }
                        }
                        free(host_queries);
                }

                pthread_mutex_lock(&global_lock);
                statistics.insert_rows += queued_values;
                statistics.insert_queries += queued_queries;
                statistics.dropped_queries += dropped_queries;
                pthread_mutex_unlock(&global_lock);

                // Note how long it took.
                end = time(NULL);

                // Prepare for next iteration.
                iterations++;
        }
        return NULL;
}

void calculate_rate(time_t prev_time, unsigned long long prev_counter, time_t cur_time, unsigned long long cur_counter, int bits, unsigned long long *counter_diff, unsigned *rate)
{
        if (bits == 0) {
                // It's a gauge so just return the value as both counter diff and rate.
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
                        // We seem to have a wrap.
                        // Wrap it back to find the correct rate.
                        if (bits == 64)
                                *counter_diff += 18446744073709551615ull + 1;   // 2^64-1 + 1
                        else
                                *counter_diff += 4294967296ull; // 2^32
                }
                // Return the calculated rate.
                *rate = (unsigned) (*counter_diff / time_diff);
        }
}

struct clinsert **get_clinserts(struct queryhost *host)
{
        struct clinsert **inserts = (struct clinsert **) xmalloc(sizeof(struct clinsert *) * MAX_TABLES);
        memset(inserts, 0, sizeof(struct clinsert *) * MAX_TABLES);

        // Start a new SNMP session.
        unsigned snmp_fail = 0, snmp_success = 0;
        struct clsnmp_session *session = clsnmp_session_create(host->host, host->community, host->snmpver);
        if (session) {
                int errors = 0;
                // Iterate over all targets in the host.
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
                                                clinsert_push_value(insert, row->id, counter_diff, rate, dtime);
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
