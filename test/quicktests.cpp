#include "UnitTest++.h"
#include "types.h"
#include "query.h"
#include "queryablehost.h"
#include "rtgconf.h"
#include "rtgtargets.h"

SUITE(QuickTests)
{
TEST(parse_example)
{
        RTGConf conf("test/example-rtg.conf");
        CHECK_EQUAL(30u, conf.interval);
        CHECK_EQUAL(2u, conf.threads);
        CHECK_EQUAL("rtguser", conf.dbuser.c_str());
        CHECK_EQUAL("password", conf.dbpass.c_str());
        CHECK_EQUAL("sql-server", conf.dbhost.c_str());
        CHECK_EQUAL("rtgdb", conf.database.c_str());
}

TEST(parse_example_1)
{
        RTGConf conf("test/example-rtg.conf");
        RTGTargets hosts("test/example-targets.cfg", conf);
        CHECK_EQUAL((size_t)2, hosts.size()); // Two hosts
        CHECK_EQUAL((size_t)2, hosts[0].rows.size()); // Two rows for host one
        CHECK_EQUAL((size_t)2, hosts[1].rows.size()); // Two rows for host two
}

TEST(parse_example_2)
{
        RTGConf conf("test/example-rtg.conf");
        RTGTargets hosts("test/example-targets.cfg", conf);
        CHECK_EQUAL("172.16.1.1", hosts[0].host.c_str());
        CHECK_EQUAL("172.16.1.2", hosts[1].host.c_str());
        CHECK_EQUAL("c0mmun1ty", hosts[0].community.c_str());
        CHECK_EQUAL("f00barb4z", hosts[1].community.c_str());
        CHECK_EQUAL(1, hosts[0].snmpver);
        CHECK_EQUAL(2, hosts[1].snmpver);
}

TEST(parse_example_3)
{
        RTGConf conf("test/example-rtg.conf");
        RTGTargets hosts("test/example-targets.cfg", conf);
        CHECK_EQUAL(".1.3.6.1.2.1.2.2.1.16.1001002", hosts[0].rows[0].oid.c_str());
        CHECK_EQUAL("ifOutOctets_362", hosts[0].rows[0].table.c_str());
        CHECK_EQUAL(4309u, hosts[0].rows[0].id);
        CHECK_EQUAL(57120000u/conf.interval, hosts[0].rows[0].speed);
        CHECK_EQUAL(32u, hosts[0].rows[0].bits);
}

TEST(zero_rate_with_32_bits)
{
        QueryHost host;
        ResultCache cache;
        QueryableHost qh(host, cache);

        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        uint64_t prev_counter = 1000000;
        uint64_t cur_counter = prev_counter;
        std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32);
        CHECK_EQUAL(0u, rate.first);
        CHECK_EQUAL(0u, rate.second);
}

TEST(one_kbps_rate_with_32_bits)
{
        QueryHost host;
        ResultCache cache;
        QueryableHost qh(host, cache);

        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        uint64_t prev_counter = 1000000;
        uint64_t cur_counter = 1000000 + 60 * 1000 / 8;
        std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32);
        CHECK_EQUAL(60u * 1000/8, rate.first);
        CHECK_EQUAL(1000u/8, rate.second);
}

TEST(one_kbps_counter_wrap_32_bits)
{
        QueryHost host;
        ResultCache cache;
        QueryableHost qh(host, cache);

        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        uint32_t prev_counter = 4294967000ul;
        uint32_t cur_counter = prev_counter + 60 * 1000 / 8;
        std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32);
        CHECK_EQUAL(60u * 1000/8, rate.first);
        CHECK_EQUAL(1000u/8, rate.second);
}

TEST(one_kbps_counter_wrap_64_bits)
{
        QueryHost host;
        ResultCache cache;
        QueryableHost qh(host, cache);

        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        uint64_t prev_counter = 18446744073709551000ull;
        uint64_t cur_counter = prev_counter + 60 * 1000 / 8;
        std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 64);
        CHECK_EQUAL(60u * 1000/8, rate.first);
        CHECK_EQUAL(1000u/8, rate.second);
}

TEST(gauge_unchanged)
{
        QueryHost host;
        ResultCache cache;
        QueryableHost qh(host, cache);

        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        uint64_t prev_counter = 1000000;
        uint64_t cur_counter = 1000000;
        std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0);
        CHECK_EQUAL(1000000u, rate.first);
        CHECK_EQUAL(1000000u, rate.second);
}

TEST(gauge_changed)
{
        QueryHost host;
        ResultCache cache;
        QueryableHost qh(host, cache);

        time_t cur_time = time(NULL);
        time_t prev_time = cur_time - 60;
        uint64_t prev_counter = 1000000;
        uint64_t cur_counter = 1000000 + 1000;
        std::pair<uint64_t, uint64_t> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0);
        CHECK_EQUAL(1000000u + 1000, rate.first);
        CHECK_EQUAL(1000000u + 1000, rate.second);
}

TEST(one_host)
{
        ResultCache cache;

        RTGConf conf("test/example-rtg.conf");
        RTGTargets hosts("test/example-targets.cfg", conf);
        QueryableHost qh(hosts[0], cache);
        std::map<std::string, ResultSet> rs = qh.get_all_resultsets();
        CHECK_EQUAL((size_t)1, rs.size()); // One table
        ResultSet set = rs["ifOutOctets_362"];
        CHECK_EQUAL((size_t)2, set.rows.size()); // Two rows
        CHECK_EQUAL(4309, set.rows[0].id);
        CHECK_EQUAL(4310, set.rows[1].id);
}
}
