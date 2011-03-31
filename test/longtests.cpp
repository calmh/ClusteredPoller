#include "UnitTest++.h"
#include "queryablehost.h"
#include "rtgconf.h"
#include "rtgtargets.h"

extern "C" void mock_set_speed(unsigned int newspeed); // From snmp-mock.cpp

SUITE(LongTests)
{
        TEST(MeasureOneHostsAt10MbpsForTenSeconds) {
                mock_set_speed(1000000 / 8);
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/example-targets.cfg", conf);

                char** queries = get_inserts(hosts->hosts[0]);
                unsigned queries_size;
                for (queries_size = 0; queries[queries_size]; queries_size++);

                CHECK_EQUAL(0u, queries_size); // No inserts first iteration

                sleep(10);

                queries = get_inserts(hosts->hosts[0]);
                for (queries_size = 0; queries[queries_size]; queries_size++);

                CHECK_EQUAL(1u, queries_size); // One insert next iteration
                char* pos = strstr(queries[0], ", 1250000, 125000)");
                CHECK(pos != NULL);
                pos = strstr(pos + 1, ", 1250000, 125000)");
                CHECK(pos != NULL);
        }

        TEST(MeasureOneHostAt100MbpsForOneInterval) {
                mock_set_speed(100000000 / 8);
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/example-targets.cfg", conf);

                char** queries = get_inserts(hosts->hosts[0]);
                unsigned queries_size;
                for (queries_size = 0; queries[queries_size]; queries_size++);

                CHECK_EQUAL(0u, queries_size); // No inserts first iteration

                sleep(conf->interval);

                queries = get_inserts(hosts->hosts[0]);
                for (queries_size = 0; queries[queries_size]; queries_size++);
                CHECK_EQUAL(0u, queries_size); // No inserts next iteration due to too high speed
        }
}
