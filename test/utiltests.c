#include <string.h>
#include "cutest.h"
#include "cllog.h"
#include "clgstr.h"

void TestGrowingString(CuTest *ct)
{
        struct clgstr *gs = clgstr_create(8);
        clgstr_append(gs, "str12345");
        clgstr_append(gs, "str34567890");
        clgstr_append(gs, "test");
        CuAssertStrEquals(ct, "str12345str34567890test", gs->string);
        CuAssertIntEquals(ct, 23, gs->length);
}

CuSuite *CuGetUtilSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestGrowingString);

        return suite;
}

