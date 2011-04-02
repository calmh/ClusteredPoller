#include <stdio.h>

#include "rtgtargets.h"
#include "queryablehost.h"
#include "globals.h"
#include "util.h"
#include "snmp.h"
#include "gstring.h"

#define MAXERRORSPERHOST 3
#define MAX_TABLES 32

db_insert *db_insert_create(char *table)
{
        db_insert *insert = (db_insert *) malloc(sizeof(db_insert));
        if (!insert)
                return NULL;
        insert->table = table;
        insert->values = (db_insert_value *) malloc(sizeof(db_insert_value) * 8);
        insert->allocated_space = 8;
        insert->nvalues = 0;
        return insert;
}

void db_insert_free(db_insert *insert)
{
        free(insert->values);
        free(insert);
}

void db_insert_push_value(db_insert *insert, unsigned id, unsigned long long counter, unsigned rate, time_t dtime)
{
        if (insert->nvalues == insert->allocated_space) {
                unsigned new_size = insert->allocated_space * 1.5;
                insert->values = (db_insert_value *) realloc(insert->values, sizeof(db_insert_value) * new_size);
                insert->allocated_space = new_size;
        }

        insert->values[insert->nvalues].id = id;
        insert->values[insert->nvalues].counter = counter;
        insert->values[insert->nvalues].rate = rate;
        insert->values[insert->nvalues].dtime = dtime;
        insert->nvalues++;
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
                                *counter_diff += 18446744073709551615ull + 1; // 2^64-1 + 1
                        else
                                *counter_diff += 4294967296ull; // 2^32
                }
                // Return the calculated rate.
                *rate = (unsigned) (*counter_diff / time_diff);
        }
}

db_insert *db_insert_for_table(db_insert **inserts, char *table)
{
        int i;
        for (i = 0; i < MAX_TABLES && inserts[i]; i++) {
                if (!strcmp(inserts[i]->table, table)) {
                        return inserts[i];
                }
        }
        inserts[i] = db_insert_create(table);
        return inserts[i];
}

db_insert **get_db_inserts(queryhost *host)
{
        db_insert **inserts = (db_insert **) malloc(sizeof(db_insert *) * MAX_TABLES);
        memset(inserts, 0, sizeof(db_insert *) * MAX_TABLES);

        // Start a new SNMP session.
        clsnmp_session *session = clsnmp_session_create(host->host, host->community, host->snmpver);
        if (session) {
                int errors = 0;
                // Iterate over all targets in the host.
                unsigned i;
                for (i = 0; i < host->nrows; i++) {
                        queryrow *row = host->rows[i];

                        unsigned long long counter;
                        time_t dtime;
                        int success = clsnmp_get(session, row->oid, &counter, &dtime);
                        if (success) {
                                if (row->bits == 0 || row->cached_time) {
                                        unsigned long long counter_diff;
                                        unsigned rate;
                                        calculate_rate(row->cached_time, row->cached_counter, dtime, counter, row->bits, &counter_diff, &rate);
                                        if (rate < row->speed) {
                                                db_insert *insert = db_insert_for_table(inserts, row->table);
                                                db_insert_push_value(insert, row->id, counter_diff, rate, dtime);
                                        }
                                }
                                row->cached_time = dtime;
                                row->cached_counter = counter;
                        } else {
                                cllog(1, "SNMP get for %s OID %s failed.", host->host, row->oid);
                                errors++;
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
        return inserts;
}

char *build_insert_query(db_insert *insert)
{
        gstr *gs = gstr_create(64);
        gstr_append(gs, "INSERT INTO ");
        gstr_append(gs, insert->table);
        gstr_append(gs, " (id, dtime, counter, rate) VALUES ");

        int rows = 0;
        char buffer[16];
        unsigned i;
        for (i = 0; i < insert->nvalues; i++) {
                if (allow_db_zero || insert->values[i].rate) {
                        if (rows > 0)
                                gstr_append(gs, ", ");
                        gstr_append(gs, "(");
                        snprintf(buffer, 15, "%u", insert->values[i].id);
                        gstr_append(gs, buffer);
                        gstr_append(gs, ", FROM_UNIXTIME(");
                        snprintf(buffer, 15, "%lu", insert->values[i].dtime);
                        gstr_append(gs, buffer);
                        gstr_append(gs, "), ");
                        snprintf(buffer, 15, "%llu", insert->values[i].counter);
                        gstr_append(gs, buffer);
                        gstr_append(gs, ", ");
                        snprintf(buffer, 15, "%u", insert->values[i].rate);
                        gstr_append(gs, buffer);
                        gstr_append(gs, ")");
                        rows++;
                }
        }
        if (rows > 0) {
                // Update the statistics
                pthread_mutex_lock(&global_lock);
                stat_inserts += rows;
                stat_queries++;
                pthread_mutex_unlock(&global_lock);
                char *return_str = strdup(gs->string);
                gstr_free(gs);
                return return_str;
        } else {
                gstr_free(gs);
                return NULL;
        }
}

unsigned num_inserts(db_insert **inserts)
{
        int count;
        for (count = 0; inserts[count] && count < MAX_TABLES; count++);
        return count;
}

char **get_inserts(queryhost *host)
{
        db_insert **inserts = get_db_inserts(host);
        unsigned n_inserts = num_inserts(inserts);

        /* Space for (at most) one insert query per table, plus an end marker. */
        char **queries = (char **) malloc(sizeof(char *) * n_inserts + 1);

        int i, j = 0;
        for (i = 0; i < n_inserts; i++) {
                char *query = build_insert_query(inserts[i]);
                if (query)
                        queries[j++] = query;
                db_insert_free(inserts[i]);
        }
        queries[j] = 0; /* End marker */

        free(inserts);
        return queries;
}
