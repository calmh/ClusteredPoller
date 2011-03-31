#include "UnitTest++.h"
#include "queryablehost.h"
#include "rtgconf.h"
#include "rtgtargets.h"

#include <string>

using namespace std;

SUITE(QuickTests)
{
        TEST(ZeroRateWith32BitsCounter) {
                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 1000000;
                unsigned long long cur_counter = prev_counter;

                unsigned long long counter_diff;
                unsigned rate;
                calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32, &counter_diff, &rate);

                CHECK_EQUAL(0u, counter_diff);
                CHECK_EQUAL(0u, rate);
        }

        TEST(OneKbpsRateWith32BitsCounter) {
                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 1000000;
                unsigned long long cur_counter = 1000000 + 60 * 1000 / 8;

                unsigned long long counter_diff;
                unsigned rate;
                calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32, &counter_diff, &rate);

                CHECK_EQUAL(60u * 1000/8, counter_diff);
                CHECK_EQUAL(1000u/8, rate);
        }

        TEST(OneKbpsRateWith32BitsCounterThatWraps) {
                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned int prev_counter = 4294967000u;
                unsigned int cur_counter = prev_counter + 60 * 1000 / 8;

                unsigned long long counter_diff;
                unsigned rate;
                calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32, &counter_diff, &rate);

                CHECK_EQUAL(60u * 1000/8, counter_diff);
                CHECK_EQUAL(1000u/8, rate);
        }

        TEST(OneKbpsRateWith64BitsCounterThatWraps) {
                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 18446744073709551000ull;
                unsigned long long cur_counter = prev_counter + 60 * 1000 / 8;

                unsigned long long counter_diff;
                unsigned rate;
                calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 64, &counter_diff, &rate);

                CHECK_EQUAL(60u * 1000/8, counter_diff);
                CHECK_EQUAL(1000u/8, rate);
        }

        TEST(GaugeValueUnchanged) {
                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 1000000;
                unsigned long long cur_counter = 1000000;

                unsigned long long counter_diff;
                unsigned rate;
                calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0, &counter_diff, &rate);

                CHECK_EQUAL(1000000u, counter_diff);
                CHECK_EQUAL(1000000u, rate);
        }

        TEST(GaugeValueChanged) {
                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 1000000;
                unsigned long long cur_counter = 1000000 + 1000;

                unsigned long long counter_diff;
                unsigned rate;
                calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0, &counter_diff, &rate);

                CHECK_EQUAL(1000000u + 1000, counter_diff);
                CHECK_EQUAL(1000000u + 1000, rate);
        }

        TEST(ResultSetForOneHost) {
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/example-targets.cfg", conf);

                db_insert** inserts = get_db_inserts(hosts->hosts[0]);
                sleep(1);
                inserts = get_db_inserts(hosts->hosts[0]);

                CHECK(NULL != inserts[0]); // One table
                CHECK(NULL == inserts[1]); // Not two tables
                CHECK_EQUAL("ifOutOctets_362", inserts[0]->table); // Two rows
                CHECK_EQUAL(2u, inserts[0]->nvalues); // Two rows
                CHECK_EQUAL(4309u, inserts[0]->values[0].id);
                CHECK_EQUAL(4310u, inserts[0]->values[1].id);
        }
}
