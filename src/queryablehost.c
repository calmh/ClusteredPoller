#include <stdio.h>

#include "clgstr.h"
#include "cllog.h"
#include "clinsert.h"
#include "clsnmp.h"
#include "globals.h"
#include "queryablehost.h"
#include "rtgtargets.h"

#define MAXERRORSPERHOST 3

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
        struct clinsert **inserts = (struct clinsert **) malloc(sizeof(struct clinsert *) * MAX_TABLES);
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
