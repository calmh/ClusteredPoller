#include "CppUnitLite/TestHarness.h"
#include "types.h"
#include "query.h"

int main()
{
    TestResult tr;
    TestRegistry::runAllTests(tr);

  return 0;
}

TEST(RTGConf, example)
{
	RTGConf conf = read_rtg_conf("example-rtg.conf");
	LONGS_EQUAL(600, conf.interval);
	LONGS_EQUAL(2, conf.threads);
	STRINGS_EQUAL("rtguser", conf.dbuser.c_str());
	STRINGS_EQUAL("password", conf.dbpass.c_str());
	STRINGS_EQUAL("sql-server", conf.dbhost.c_str());
	STRINGS_EQUAL("rtgdb", conf.database.c_str());
}
