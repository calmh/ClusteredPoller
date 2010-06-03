#include "UnitTest++.h"
#include "rtgconf.h"

using namespace std;

SUITE(QuickTests)
{
        TEST(ParseConfigurationVariables) {
                RTGConf conf("test/example-rtg.conf");
                CHECK_EQUAL(30u, conf.interval);
                CHECK_EQUAL(2u, conf.threads);
                CHECK_EQUAL("rtguser", conf.dbuser);
                CHECK_EQUAL("password", conf.dbpass);
                CHECK_EQUAL("sql-server", conf.dbhost);
                CHECK_EQUAL("rtgdb", conf.database);
        }
}
