#include <string>

#include "UnitTest++.h"
#include "util.h"
#include "gstring.h"

using namespace std;

SUITE(QuickTests)
{
        TEST(GrowingString) {
                gstr* gs = gstr_create(8);
                gstr_append(gs, "str12345");
                gstr_append(gs, "str34567890");
                gstr_append(gs, "test");
                CHECK_EQUAL("str12345str34567890test", gs->string);
                CHECK_EQUAL(23u, gs->length);
        }
}
