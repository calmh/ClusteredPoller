#include <stdlib.h>
#include "cutest.h"
#include "rtgconf.h"

void TestParseConfigurationVariables(CuTest *tc)
{
        rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        CuAssertIntEquals(tc, 30, conf->interval);
        CuAssertIntEquals(tc, 2, conf->threads);
        CuAssertStrEquals(tc, "rtguser", conf->dbuser);
        CuAssertStrEquals(tc, "password", conf->dbpass);
        CuAssertStrEquals(tc, "sql-server", conf->dbhost);
        CuAssertStrEquals(tc, "rtgdb", conf->database);
        rtgconf_free(conf);
}

CuSuite *CuGetRtgConfSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestParseConfigurationVariables);

        return suite;
}

