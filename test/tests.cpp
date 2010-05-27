#include "TestHarness.h"
#include "types.h"
#include "query.h"
#include "queryablehost.h"
#include "rtgconf.h"
#include "rtgtargets.h"

void mock_set_speed(unsigned int newspeed); // From snmp-mock.cpp

int main()
{
    TestResult tr;
    TestRegistry::runAllTests(tr);

  return 0;
}

TEST(RTGConf, parse_example)
{
	RTGConf conf("example-rtg.conf");
	LONGS_EQUAL(30, conf.interval);
	LONGS_EQUAL(2, conf.threads);
	STRINGS_EQUAL("rtguser", conf.dbuser.c_str());
	STRINGS_EQUAL("password", conf.dbpass.c_str());
	STRINGS_EQUAL("sql-server", conf.dbhost.c_str());
	STRINGS_EQUAL("rtgdb", conf.database.c_str());
}

TEST(Targets, parse_example_1)
{
	RTGConf conf("example-rtg.conf");
	RTGTargets hosts("example-targets.cfg", conf);
	LONGS_EQUAL(2, hosts.size()); // Two hosts
	LONGS_EQUAL(2, hosts[0].rows.size()); // Two rows for host one
	LONGS_EQUAL(2, hosts[1].rows.size()); // Two rows for host two
}

TEST(Targets, parse_example_2)
{
	RTGConf conf("example-rtg.conf");
	RTGTargets hosts("example-targets.cfg", conf);
	STRINGS_EQUAL("172.16.1.1", hosts[0].host.c_str());
	STRINGS_EQUAL("172.16.1.2", hosts[1].host.c_str());
	STRINGS_EQUAL("c0mmun1ty", hosts[0].community.c_str());
	STRINGS_EQUAL("f00barbaz", hosts[1].community.c_str());
	LONGS_EQUAL(1, hosts[0].snmpver);
	LONGS_EQUAL(2, hosts[1].snmpver);
}

TEST(Targets, parse_example_3)
{
	RTGConf conf("example-rtg.conf");
	RTGTargets hosts("example-targets.cfg", conf);
	STRINGS_EQUAL(".1.3.6.1.2.1.2.2.1.16.1001002", hosts[0].rows[0].oid.c_str());
	STRINGS_EQUAL("ifOutOctets_362", hosts[0].rows[0].table.c_str());
	LONGS_EQUAL(4309, hosts[0].rows[0].id);
	UINT64_EQUAL(285600000, hosts[0].rows[0].speed);
	LONGS_EQUAL(32, hosts[0].rows[0].bits);
}

TEST(CalculateRate, zero_rate_with_32_bits)
{
	QueryHost host;
	ResultCache cache;
	QueryableHost qh(host, cache);

	time_t cur_time = time(NULL);
	time_t prev_time = cur_time - 60;
	uint64_t prev_counter = 1000000;
	uint64_t cur_counter = prev_counter;
	std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32);
	UINT64_EQUAL(0, rate.first);
	UINT64_EQUAL(0, rate.second);
}

TEST(CalculateRate, one_kbps_rate_with_32_bits)
{
	QueryHost host;
	ResultCache cache;
	QueryableHost qh(host, cache);

	time_t cur_time = time(NULL);
	time_t prev_time = cur_time - 60;
	uint64_t prev_counter = 1000000;
	uint64_t cur_counter = 1000000 + 60 * 1000 / 8;
	std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32);
	UINT64_EQUAL(60 * 1000/8, rate.first);
	UINT64_EQUAL(1000/8, rate.second);
}

TEST(CalculateRate, one_kbps_counter_wrap_32_bits)
{
	QueryHost host;
	ResultCache cache;
	QueryableHost qh(host, cache);

	time_t cur_time = time(NULL);
	time_t prev_time = cur_time - 60;
	uint32_t prev_counter = 4294967000ul;
	uint32_t cur_counter = prev_counter + 60 * 1000 / 8;
	std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32);
	UINT64_EQUAL(60 * 1000/8, rate.first);
	UINT64_EQUAL(1000/8, rate.second);
}

TEST(CalculateRate, one_kbps_counter_wrap_64_bits)
{
	QueryHost host;
	ResultCache cache;
	QueryableHost qh(host, cache);

	time_t cur_time = time(NULL);
	time_t prev_time = cur_time - 60;
	uint64_t prev_counter = 18446744073709551000ull;
	uint64_t cur_counter = prev_counter + 60 * 1000 / 8;
	std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 64);
	UINT64_EQUAL(60 * 1000/8, rate.first);
	UINT64_EQUAL(1000/8, rate.second);
}

TEST(CalculateRate, gauge_unchanged)
{
	QueryHost host;
	ResultCache cache;
	QueryableHost qh(host, cache);

	time_t cur_time = time(NULL);
	time_t prev_time = cur_time - 60;
	uint64_t prev_counter = 1000000;
	uint64_t cur_counter = 1000000;
	std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0);
	UINT64_EQUAL(1000000, rate.first);
	UINT64_EQUAL(1000000, rate.second);
}

TEST(CalculateRate, gauge_changed)
{
	QueryHost host;
	ResultCache cache;
	QueryableHost qh(host, cache);

	time_t cur_time = time(NULL);
	time_t prev_time = cur_time - 60;
	uint64_t prev_counter = 1000000;
	uint64_t cur_counter = 1000000 + 1000;
	std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0);
	UINT64_EQUAL(1000000 + 1000, rate.first);
	UINT64_EQUAL(1000000 + 1000, rate.second);
}

TEST(Query, one_host)
{
	ResultCache cache;

	RTGConf conf("example-rtg.conf");
	RTGTargets hosts("example-targets.cfg", conf);
	QueryableHost qh(hosts[0], cache);
	std::map<std::string, ResultSet> rs = qh.get_all_resultsets();
	LONGS_EQUAL(1, rs.size()); // One table
	ResultSet set = rs["ifOutOctets_362"];
	LONGS_EQUAL(2, set.rows.size()); // Two rows
	LONGS_EQUAL(4309, set.rows[0].id);
	LONGS_EQUAL(4310, set.rows[1].id);
}

#ifdef LONGTESTS
TEST(ProcessHost, one_host_one_Mbps_10_secs)
{
	mock_set_speed(1000000 / 8);
	RTGConf conf("example-rtg.conf");
	RTGTargets hosts("example-targets.cfg", conf);
	ResultCache cache;
	QueryableHost qh(hosts[0], cache);
	std::vector<std::string> queries = qh.get_inserts();
	LONGS_EQUAL(0, queries.size()); // No inserts first iteration
	sleep(10);
	queries = qh.get_inserts();
	LONGS_EQUAL(1, queries.size()); // One insert next iteration
	// std::cerr << queries[0] << std::endl;
	size_t pos = queries[0].find(", 1250000, 125000)");
	CHECK(pos != std::string::npos);
	pos = queries[0].find(", 1250000, 125000)", pos+1);
	CHECK(pos != std::string::npos);
}

TEST(ProcessHost, one_host_hundred_Mbps_one_interval)
{
	mock_set_speed(100e6 / 8);
	RTGConf conf("example-rtg.conf");
	RTGTargets hosts("example-targets.cfg", conf);
	ResultCache cache;
	QueryableHost qh(hosts[0], cache);
	std::vector<std::string> queries = qh.get_inserts();
	LONGS_EQUAL(0, queries.size()); // No inserts first iteration
	sleep(conf.interval);
	queries = qh.get_inserts();
	LONGS_EQUAL(0, queries.size()); // No inserts next iteration due to too high speed
}
#endif
