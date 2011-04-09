#include <stdio.h>

#include "rtgtargets.h"
#include "queryablehost.h"
#include "globals.h"
#include "cllog.h"
#include "clsnmp.h"
#include "clgstr.h"

#define MAXERRORSPERHOST 3
#define MAX_TABLES 32

struct db_insert *db_insert_create(char *table)
{
        struct db_insert *insert = (struct db_insert *) malloc(sizeof(struct db_insert));
        if (!insert)
                return NULL;
        insert->table = table;
        insert->values = (struct db_insert_value *) malloc(sizeof(struct db_insert_value) * 8);
        insert->allocated_space = 8;
        insert->nvalues = 0;
        return insert;
}

void db_insert_free(struct db_insert *insert)
{
        free(insert->values);
        free(insert);
}

void db_insert_push_value(struct db_insert *insert, unsigned id, unsigned long long counter, unsigned rate, time_t dtime)
{
        if (insert->nvalues == insert->allocated_space) {
                unsigned new_size = insert->allocated_space * 1.5;
                insert->values = (struct db_insert_value *) realloc(insert->values, sizeof(struct db_insert_value) * new_size);
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
                                *counter_diff += 18446744073709551615ull + 1;   // 2^64-1 + 1
                        else
                                *counter_diff += 4294967296ull; // 2^32
                }
                // Return the calculated rate.
                *rate = (unsigned) (*counter_diff / time_diff);
        }
}

struct db_insert *db_insert_for_table(struct db_insert **inserts, char *table)
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

struct db_insert **get_db_inserts(struct queryhost *host)
{
        struct db_insert **inserts = (struct db_insert **) malloc(sizeof(struct db_insert *) * MAX_TABLES);
        memset(inserts, 0, sizeof(struct db_insert *) * MAX_TABLES);

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
                                                struct db_insert *insert = db_insert_for_table(inserts, row->table);
                                                db_insert_push_value(insert, row->id, counter_diff, rate, dtime);
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

unsigned num_inserts(struct db_insert **inserts)
{
        int count;
        for (count = 0; inserts[count] && count < MAX_TABLES; count++) ;
        return count;
}
