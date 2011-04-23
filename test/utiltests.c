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
        CuAssertStrEquals(ct, "str12345str34567890test", clgstr_string(gs));
        CuAssertIntEquals(ct, 23, clgstr_length(gs));
}

CuSuite *CuGetUtilSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestGrowingString);

        return suite;
}
