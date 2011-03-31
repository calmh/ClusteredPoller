#include <string>

#include "types.h"
#include "UnitTest++.h"
#include "rtgconf.h"
#include "rtgtargets.h"

using namespace std;

SUITE(QuickTests)
{
        TEST(ParseNonexistentTargets) {
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/none.cfg", conf);
                CHECK_EQUAL((size_t)0, hosts->nhosts);
        }

        TEST(ParseNewStyleTargets) {
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/example-targets.cfg", conf);
                CHECK_EQUAL((size_t)2, hosts->nhosts); // Two hosts
                CHECK_EQUAL((size_t)2, hosts->hosts[0]->nrows); // Two rows for host one
                CHECK_EQUAL((size_t)2, hosts->hosts[1]->nrows); // Two rows for host two
                CHECK_EQUAL(2u, hosts->nhosts);
                CHECK_EQUAL(4u, hosts->ntargets);
        }

        TEST(ParseOldStyleTargets) {
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/oldstyle-targets.cfg", conf);
                CHECK_EQUAL((size_t)2, hosts->nhosts); // Two hosts
                CHECK_EQUAL((size_t)8, hosts->hosts[0]->nrows); // Eight rows for host one
                CHECK_EQUAL((size_t)7, hosts->hosts[1]->nrows); // Seven rows for host two
                CHECK_EQUAL(2u, hosts->nhosts);
                CHECK_EQUAL(8u+7u, hosts->ntargets);
        }

        TEST(ParseNewStyleTargetsHost) {
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/example-targets.cfg", conf);
                CHECK_EQUAL("172.16.1.1", hosts->hosts[0]->host);
                CHECK_EQUAL("172.16.1.2", hosts->hosts[1]->host);
                CHECK_EQUAL("c0mmun1ty", hosts->hosts[0]->community);
                CHECK_EQUAL("f00barb4z", hosts->hosts[1]->community);
                CHECK_EQUAL(1, hosts->hosts[0]->snmpver);
                CHECK_EQUAL(2, hosts->hosts[1]->snmpver);
        }

        TEST(ParseOldStyleTargetsHost) {
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/oldstyle-targets.cfg", conf);
                CHECK_EQUAL("172.16.17.18", hosts->hosts[0]->host);
                CHECK_EQUAL("172.16.17.20", hosts->hosts[1]->host);
                CHECK_EQUAL("public", hosts->hosts[0]->community);
                CHECK_EQUAL("otherc", hosts->hosts[1]->community);
                CHECK_EQUAL(2, hosts->hosts[0]->snmpver);
                CHECK_EQUAL(2, hosts->hosts[1]->snmpver);
        }

        TEST(ParseNewsStyleTargetsRow) {
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/example-targets.cfg", conf);
                CHECK_EQUAL(".1.3.6.1.2.1.2.2.1.16.1001002", hosts->hosts[0]->rows[0]->oid);
                CHECK_EQUAL("ifOutOctets_362", hosts->hosts[0]->rows[0]->table);
                CHECK_EQUAL(4309u, hosts->hosts[0]->rows[0]->id);
                CHECK_EQUAL(57120000u/conf->interval, hosts->hosts[0]->rows[0]->speed);
                CHECK_EQUAL(32u, hosts->hosts[0]->rows[0]->bits);
        }

        TEST(ParseOldStyleTargetsRow) {
                rtgconf* conf = rtgconf_create("test/example-rtg.conf");
                rtgtargets* hosts = rtgtargets_parse("test/oldstyle-targets.cfg", conf);
                CHECK_EQUAL("1.3.6.1.2.1.2.2.1.10.13", hosts->hosts[0]->rows[0]->oid);
                CHECK_EQUAL("data", hosts->hosts[0]->rows[0]->table);
                CHECK_EQUAL(55u, hosts->hosts[0]->rows[0]->id);
                CHECK_EQUAL((unsigned)10e9/8/conf->interval, hosts->hosts[0]->rows[0]->speed);
                CHECK_EQUAL(32u, hosts->hosts[0]->rows[0]->bits);
        }
}
