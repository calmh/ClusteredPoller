#include "UnitTest++.h"
#include "types.h"
#include "queryablehost.h"
#include "rtgconf.h"
#include "rtgtargets.h"

#include <string>

using namespace std;

SUITE(QuickTests)
{
        TEST(ZeroRateWith32BitsCounter) {
                QueryHost host;
                ResultCache cache;
                QueryableHost qh(host, cache);

                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 1000000;
                unsigned long long cur_counter = prev_counter;
                std::pair<unsigned long long, unsigned long long> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32);
                CHECK_EQUAL(0u, rate.first);
                CHECK_EQUAL(0u, rate.second);
        }

        TEST(OneKbpsRateWith32BitsCounter) {
                QueryHost host;
                ResultCache cache;
                QueryableHost qh(host, cache);

                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 1000000;
                unsigned long long cur_counter = 1000000 + 60 * 1000 / 8;
                std::pair<unsigned long long, unsigned long long> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32);
                CHECK_EQUAL(60u * 1000/8, rate.first);
                CHECK_EQUAL(1000u/8, rate.second);
        }

        TEST(OneKbpsRateWith32BitsCounterThatWraps) {
                QueryHost host;
                ResultCache cache;
                QueryableHost qh(host, cache);

                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned int prev_counter = 4294967000u;
                unsigned int cur_counter = prev_counter + 60 * 1000 / 8;
                std::pair<unsigned long long, unsigned long long> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 32);
                CHECK_EQUAL(60u * 1000/8, rate.first);
                CHECK_EQUAL(1000u/8, rate.second);
        }

        TEST(OneKbpsRateWith64BitsCounterThatWraps) {
                QueryHost host;
                ResultCache cache;
                QueryableHost qh(host, cache);

                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 18446744073709551000ull;
                unsigned long long cur_counter = prev_counter + 60 * 1000 / 8;
                std::pair<unsigned long long, unsigned long long> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 64);
                CHECK_EQUAL(60u * 1000/8, rate.first);
                CHECK_EQUAL(1000u/8, rate.second);
        }

        TEST(GaugeValueUnchanged) {
                QueryHost host;
                ResultCache cache;
                QueryableHost qh(host, cache);

                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 1000000;
                unsigned long long cur_counter = 1000000;
                std::pair<unsigned long long, unsigned long long> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0);
                CHECK_EQUAL(1000000u, rate.first);
                CHECK_EQUAL(1000000u, rate.second);
        }

        TEST(GaugeValueChanged) {
                QueryHost host;
                ResultCache cache;
                QueryableHost qh(host, cache);

                time_t cur_time = time(NULL);
                time_t prev_time = cur_time - 60;
                unsigned long long prev_counter = 1000000;
                unsigned long long cur_counter = 1000000 + 1000;
                std::pair<unsigned long long, unsigned long long> rate = qh.calculate_rate(prev_time, prev_counter, cur_time, cur_counter, 0);
                CHECK_EQUAL(1000000u + 1000, rate.first);
                CHECK_EQUAL(1000000u + 1000, rate.second);
        }

        TEST(ResultSetForOneHost) {
                ResultCache cache;

                RTGConf conf("test/example-rtg.conf");
                RTGTargets hosts("test/example-targets.cfg", &conf);
                QueryableHost qh(hosts[0], cache);
                std::map<std::string, ResultSet> rs = qh.get_all_resultsets();
                CHECK_EQUAL((size_t)1, rs.size()); // One table
                ResultSet set = rs["ifOutOctets_362"];
                CHECK_EQUAL((size_t)2, set.rows.size()); // Two rows
                CHECK_EQUAL(4309, set.rows[0].id);
                CHECK_EQUAL(4310, set.rows[1].id);
        }
}
