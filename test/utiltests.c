#include <string.h>
#include "cutest.h"
#include "util.h"
#include "gstring.h"

void TestGrowingString(CuTest *ct)
{
        gstr *gs = gstr_create(8);
        gstr_append(gs, "str12345");
        gstr_append(gs, "str34567890");
        gstr_append(gs, "test");
        CuAssertStrEquals(ct, "str12345str34567890test", gs->string);
        CuAssertIntEquals(ct, 23, gs->length);
}

CuSuite *CuGetUtilSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestGrowingString);

        return suite;
}

