#include "UnitTest++.h"
#include "types.h"
#include "query.h"
#include "queryablehost.h"
#include "rtgconf.h"
#include "rtgtargets.h"
#include "util.h"
#include <string>
using namespace std;

SUITE(QuickTests)
{
        TEST(ParseConfigurationVariables)
        {
                RTGConf conf("test/example-rtg.conf");
                CHECK_EQUAL(30u, conf.interval);
                CHECK_EQUAL(2u, conf.threads);
                CHECK_EQUAL("rtguser", conf.dbuser.c_str());
                CHECK_EQUAL("password", conf.dbpass.c_str());
                CHECK_EQUAL("sql-server", conf.dbhost.c_str());
                CHECK_EQUAL("rtgdb", conf.database.c_str());
        }

        TEST(ParseNewStyleTargets)
        {
                RTGConf conf("test/example-rtg.conf");
                RTGTargets hosts("test/example-targets.cfg", conf);
                CHECK_EQUAL((size_t)2, hosts.size()); // Two hosts
                CHECK_EQUAL((size_t)2, hosts[0].rows.size()); // Two rows for host one
                CHECK_EQUAL((size_t)2, hosts[1].rows.size()); // Two rows for host two
        }

        TEST(ParseOldStyleTargets)
        {
                RTGConf conf("test/example-rtg.conf");
                RTGTargets hosts("test/oldstyle-targets.cfg", conf);
                CHECK_EQUAL((size_t)2, hosts.size()); // Two hosts
                CHECK_EQUAL((size_t)8, hosts[0].rows.size()); // Two rows for host one
                CHECK_EQUAL((size_t)7, hosts[1].rows.size()); // Two rows for host two
        }

        TEST(ParseNewStyleTargetsHost)
        {
                RTGConf conf("test/example-rtg.conf");
                RTGTargets hosts("test/example-targets.cfg", conf);
                CHECK_EQUAL("172.16.1.1", hosts[0].host);
                CHECK_EQUAL("172.16.1.2", hosts[1].host);
                CHECK_EQUAL("c0mmun1ty", hosts[0].community);
                CHECK_EQUAL("f00barb4z", hosts[1].community);
                CHECK_EQUAL(1, hosts[0].snmpver);
                CHECK_EQUAL(2, hosts[1].snmpver);
        }

        TEST(ParseOldStyleTargetsHost)
        {
                RTGConf conf("test/example-rtg.conf");
                RTGTargets hosts("test/oldstyle-targets.cfg", conf);
                CHECK_EQUAL("172.16.17.18", hosts[0].host);
                CHECK_EQUAL("172.16.17.20", hosts[1].host);
                CHECK_EQUAL("public", hosts[0].community);
                CHECK_EQUAL("otherc", hosts[1].community);
                CHECK_EQUAL(2, hosts[0].snmpver);
                CHECK_EQUAL(2, hosts[1].snmpver);
        }

        TEST(ParseNewsStyleTargetsRow)
        {
                RTGConf conf("test/example-rtg.conf");
                RTGTargets hosts("test/example-targets.cfg", conf);
                CHECK_EQUAL(".1.3.6.1.2.1.2.2.1.16.1001002", hosts[0].rows[0].oid);
                CHECK_EQUAL("ifOutOctets_362", hosts[0].rows[0].table);
                CHECK_EQUAL(4309u, hosts[0].rows[0].id);
                CHECK_EQUAL(57120000u/conf.interval, hosts[0].rows[0].speed);
                CHECK_EQUAL(32u, hosts[0].rows[0].bits);
        }

        TEST(ParseOldStyleTargetsRow)
        {
                RTGConf conf("test/example-rtg.conf");
                RTGTargets hosts("test/oldstyle-targets.cfg", conf);
                CHECK_EQUAL("1.3.6.1.2.1.2.2.1.10.13", hosts[0].rows[0].oid);
                CHECK_EQUAL("data", hosts[0].rows[0].table);
                CHECK_EQUAL(55u, hosts[0].rows[0].id);
                CHECK_EQUAL((unsigned)10e9/8/conf.interval, hosts[0].rows[0].speed);
                CHECK_EQUAL(32u, hosts[0].rows[0].bits);
        }

        TEST(ZeroRateWith32BitsCounter)
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

        TEST(OneKbpsRateWith32BitsCounter)
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

        TEST(OneKbpsRateWith32BitsCounterThatWraps)
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

        TEST(OneKbpsRateWith64BitsCounterThatWraps)
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

        TEST(GaugeValueUnchanged)
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

        TEST(GaugeValueChanged)
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

        TEST(ResultSetForOneHost)
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

        TEST(UncommentOneLine)
        {
                string line("# just a comment");
                CHECK_EQUAL("", string_uncomment(line));
                line = "text before# comment";
                CHECK_EQUAL("text before", string_uncomment(line));
                line = "   text before   # comment";
                CHECK_EQUAL("text before", string_uncomment(line));
        }

        TEST(SplitOneLineIntoParts)
        {
                string line("two\twords ");
                list<string> parts = string_split(line, "\t");
                CHECK_EQUAL((size_t)2, parts.size());
                CHECK_EQUAL("two", parts.front()); parts.pop_front();
                CHECK_EQUAL("words ", parts.front());
        }

        TEST(SplitOneComplexLineIntoParts)
        {
                string line = "more words\t separated\tby tabs \t";
                list<string> parts = string_split(line, "\t");
                CHECK_EQUAL((size_t)4, parts.size());
                CHECK_EQUAL("more words", parts.front()); parts.pop_front();
                CHECK_EQUAL(" separated", parts.front()); parts.pop_front();
                CHECK_EQUAL("by tabs ", parts.front()); parts.pop_front();
                CHECK_EQUAL("", parts.front());
        }
}
