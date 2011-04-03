#include <stdlib.h>
#include "cutest.h"
#include "rtgconf.h"

void TestParseConfigurationVariables(CuTest *tc)
{
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        CuAssertIntEquals(tc, 30, conf->interval);
        CuAssertIntEquals(tc, 2, conf->threads);
        CuAssertStrEquals(tc, "Rtguser", conf->dbuser);
        CuAssertStrEquals(tc, "Password", conf->dbpass);
        CuAssertStrEquals(tc, "Sql-server", conf->dbhost);
        CuAssertStrEquals(tc, "Rtgdb", conf->database);
        rtgconf_free(conf);
}

CuSuite *CuGetRtgConfSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestParseConfigurationVariables);

        return suite;
}

