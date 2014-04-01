#include <string.h>
#include <unistd.h>

#include "clinsert.h"
#include "cutest.h"
#include "rtgconf.h"
#include "rtgtargets.h"
#include "poller.h"

void TestZeroRateWith32BitsCounter(CuTest *tc)
{
        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        unsigned long long prev_counter = 1000000;
        unsigned long long cur_counter = prev_counter;

        unsigned long long counter_diff;
        unsigned rate;
        calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32, &counter_diff, &rate);

        CuAssertIntEquals(tc, 0u, counter_diff);
        CuAssertIntEquals(tc, 0u, rate);
}

void TestOneKbpsRateWith32BitsCounter(CuTest *tc)
{
        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        unsigned long long prev_counter = 1000000;
        unsigned long long cur_counter = 1000000 + 60 * 1000 / 8;

        unsigned long long counter_diff;
        unsigned rate;
        calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32, &counter_diff, &rate);

        CuAssertIntEquals(tc, 60u * 1000 / 8, counter_diff);
        CuAssertIntEquals(tc, 1000u / 8, rate);
}

void TestOneKbpsRateWith32BitsCounterThatWraps(CuTest *tc)
{
        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        unsigned int prev_counter = 4294967000u;
        unsigned int cur_counter = prev_counter + 60 * 1000 / 8;

        unsigned long long counter_diff;
        unsigned rate;
        calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32, &counter_diff, &rate);

        CuAssertIntEquals(tc, 60u * 1000 / 8, counter_diff);
        CuAssertIntEquals(tc, 1000u / 8, rate);
}

void TestOneKbpsRateWith64BitsCounterThatWraps(CuTest *tc)
{
        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        unsigned long long prev_counter = 18446744073709551000ull;
        unsigned long long cur_counter = prev_counter + 60 * 1000 / 8;

        unsigned long long counter_diff;
        unsigned rate;
        calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 64, &counter_diff, &rate);

        CuAssertIntEquals(tc, 60u * 1000 / 8, counter_diff);
        CuAssertIntEquals(tc, 1000u / 8, rate);
}

void TestGaugeValueUnchanged(CuTest *tc)
{
        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        unsigned long long prev_counter = 1000000;
        unsigned long long cur_counter = 1000000;

        unsigned long long counter_diff;
        unsigned rate;
        calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0, &counter_diff, &rate);

        CuAssertIntEquals(tc, 1000000u, counter_diff);
        CuAssertIntEquals(tc, 1000000u, rate);
}

void TestGaugeValueChanged(CuTest *tc)
{
        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        unsigned long long prev_counter = 1000000;
        unsigned long long cur_counter = 1000000 + 1000;

        unsigned long long counter_diff;
        unsigned rate;
        calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0, &counter_diff, &rate);

        CuAssertIntEquals(tc, 1000000u + 1000, counter_diff);
        CuAssertIntEquals(tc, 1000000u + 1000, rate);
}

void TestResultSetForOneHost(CuTest *tc)
{
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/example-targets.cfg", conf);

        struct clinsert **inserts = get_clinserts(hosts->hosts[0], 3);
        sleep(1);
        inserts = get_clinserts(hosts->hosts[0], 3);

        CuAssertTrue(tc, NULL != inserts[0]);   /* One table */
        CuAssertTrue(tc, NULL == inserts[1]);   /* Not two tables */
        CuAssertStrEquals(tc, "ifOutOctets_362", inserts[0]->table);    /* Two rows */
        CuAssertIntEquals(tc, 2u, inserts[0]->nvalues); /* Two rows */
        CuAssertIntEquals(tc, 4309u, inserts[0]->values[0].id);
        CuAssertIntEquals(tc, 4310u, inserts[0]->values[1].id);
}

CuSuite *CuGetIntegrationSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestZeroRateWith32BitsCounter);
        SUITE_ADD_TEST(suite, TestOneKbpsRateWith32BitsCounter);
        SUITE_ADD_TEST(suite, TestOneKbpsRateWith32BitsCounterThatWraps);
        SUITE_ADD_TEST(suite, TestOneKbpsRateWith64BitsCounterThatWraps);
        SUITE_ADD_TEST(suite, TestGaugeValueUnchanged);
        SUITE_ADD_TEST(suite, TestGaugeValueChanged);
        SUITE_ADD_TEST(suite, TestResultSetForOneHost);

        return suite;
}
