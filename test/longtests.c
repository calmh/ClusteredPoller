#include <unistd.h>
#include <string.h>

#include "cutest.h"
#include "queryablehost.h"
#include "rtgconf.h"
#include "rtgtargets.h"
#include "globals.h"

void mock_set_speed(unsigned int newspeed);     // From snmp-mock.c

void TestMeasureOneHostsAt10MbpsForTenSeconds(CuTest *tc)
{
        use_rate_column = 1;

        mock_set_speed(1000000 / 8);
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/example-targets.cfg", conf);

        struct db_insert **queries = get_db_inserts(hosts->hosts[0]);
        unsigned queries_size;
        for (queries_size = 0; queries[queries_size]; queries_size++) ;

        CuAssertIntEquals(tc, 0, queries_size); // No inserts first iteration

        sleep(10);

        queries = get_db_inserts(hosts->hosts[0]);
        for (queries_size = 0; queries[queries_size]; queries_size++) ;

        CuAssertIntEquals(tc, 1, queries_size); // One insert next iteration
        CuAssertIntEquals(tc, 1250000, queries[0]->values[0].counter);
        CuAssertIntEquals(tc, 125000, queries[0]->values[0].rate);
}

void TestMeasureOneHostAt100MbpsForOneInterval(CuTest *tc)
{
        mock_set_speed(100000000 / 8);
        struct rtgconf *conf = rtgconf_create("test/example-rtg.conf");
        struct rtgtargets *hosts = rtgtargets_parse("test/example-targets.cfg", conf);

        struct db_insert **queries = get_db_inserts(hosts->hosts[0]);
        unsigned queries_size;
        for (queries_size = 0; queries[queries_size]; queries_size++) ;

        CuAssertIntEquals(tc, 0, queries_size); // No inserts first iteration

        sleep(conf->interval);

        queries = get_db_inserts(hosts->hosts[0]);
        for (queries_size = 0; queries[queries_size]; queries_size++) ;
        CuAssertIntEquals(tc, 0, queries_size); // No inserts next iteration due to too high speed
}

CuSuite *CuGetLongSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestMeasureOneHostsAt10MbpsForTenSeconds);
        SUITE_ADD_TEST(suite, TestMeasureOneHostAt100MbpsForOneInterval);

        return suite;
}
