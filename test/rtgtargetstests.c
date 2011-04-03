#include <string.h>
#include "cutest.h"
#include "rtgconf.h"
#include "rtgtargets.h"

void TestParseNonexistentTargets(CuTest *tc)
{
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/none.cfg", conf);
        CuAssertIntEquals(tc, 0, hosts->nhosts);
}

void TestParseNewStyleTargets(CuTest *tc)
{
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/example-targets.cfg", conf);
        CuAssertIntEquals(tc, 2, hosts->nhosts); // Two hosts
        CuAssertIntEquals(tc, 2, hosts->hosts[0]->nrows); // Two rows for host one
        CuAssertIntEquals(tc, 2, hosts->hosts[1]->nrows); // Two rows for host two
        CuAssertIntEquals(tc, 2u, hosts->nhosts);
        CuAssertIntEquals(tc, 4u, hosts->ntargets);
}

void TestParseOldStyleTargets(CuTest *tc)
{
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/oldstyle-targets.cfg", conf);
        CuAssertIntEquals(tc, 2, hosts->nhosts); // Two hosts
        CuAssertIntEquals(tc, 8, hosts->hosts[0]->nrows); // Eight rows for host one
        CuAssertIntEquals(tc, 7, hosts->hosts[1]->nrows); // Seven rows for host two
        CuAssertIntEquals(tc, 2u, hosts->nhosts);
        CuAssertIntEquals(tc, 8u+7u, hosts->ntargets);
}

void TestParseNewStyleTargetsHost(CuTest *tc)
{
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/example-targets.cfg", conf);
        CuAssertStrEquals(tc, "172.16.1.1", hosts->hosts[0]->host);
        CuAssertStrEquals(tc, "172.16.1.2", hosts->hosts[1]->host);
        CuAssertStrEquals(tc, "c0mmun1ty", hosts->hosts[0]->community);
        CuAssertStrEquals(tc, "f00barb4z", hosts->hosts[1]->community);
        CuAssertIntEquals(tc, 1, hosts->hosts[0]->snmpver);
        CuAssertIntEquals(tc, 2, hosts->hosts[1]->snmpver);
}

void TestParseOldStyleTargetsHost(CuTest *tc)
{
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/oldstyle-targets.cfg", conf);
        CuAssertStrEquals(tc, "172.16.17.18", hosts->hosts[0]->host);
        CuAssertStrEquals(tc, "172.16.17.20", hosts->hosts[1]->host);
        CuAssertStrEquals(tc, "public", hosts->hosts[0]->community);
        CuAssertStrEquals(tc, "otherc", hosts->hosts[1]->community);
        CuAssertIntEquals(tc, 2, hosts->hosts[0]->snmpver);
        CuAssertIntEquals(tc, 2, hosts->hosts[1]->snmpver);
}

void TestParseNewStyleTargetsRow(CuTest *tc)
{
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/example-targets.cfg", conf);
        CuAssertStrEquals(tc, ".1.3.6.1.2.1.2.2.1.16.1001002", hosts->hosts[0]->rows[0]->oid);
        CuAssertStrEquals(tc, "ifOutOctets_362", hosts->hosts[0]->rows[0]->table);
        CuAssertIntEquals(tc, 4309u, hosts->hosts[0]->rows[0]->id);
        CuAssertIntEquals(tc, 57120000u/conf->interval, hosts->hosts[0]->rows[0]->speed);
        CuAssertIntEquals(tc, 32u, hosts->hosts[0]->rows[0]->bits);
}

void TestParseOldStyleTargetsRow(CuTest *tc)
{
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/oldstyle-targets.cfg", conf);
        CuAssertStrEquals(tc, "1.3.6.1.2.1.2.2.1.10.13", hosts->hosts[0]->rows[0]->oid);
        CuAssertStrEquals(tc, "data", hosts->hosts[0]->rows[0]->table);
        CuAssertIntEquals(tc, 55u, hosts->hosts[0]->rows[0]->id);
        CuAssertIntEquals(tc, (unsigned)10e9/8/conf->interval, hosts->hosts[0]->rows[0]->speed);
        CuAssertIntEquals(tc, 32u, hosts->hosts[0]->rows[0]->bits);
}

CuSuite *CuGetRtgTargetsSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestParseNonexistentTargets);
        SUITE_ADD_TEST(suite, TestParseNewStyleTargets);
        SUITE_ADD_TEST(suite, TestParseOldStyleTargets);
        SUITE_ADD_TEST(suite, TestParseNewStyleTargetsHost);
        SUITE_ADD_TEST(suite, TestParseOldStyleTargetsHost);
        SUITE_ADD_TEST(suite, TestParseNewStyleTargetsRow);
        SUITE_ADD_TEST(suite, TestParseOldStyleTargetsRow);

        return suite;
}

