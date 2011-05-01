#include "cltime.h"
#include "cutest.h"
#include <time.h>

/*
void CuAssert(CuTest* tc, char* message, int condition);
void CuAssertTrue(CuTest* tc, int condition);
void CuAssertStrEquals(CuTest* tc, char* expected, char* actual);
void CuAssertIntEquals(CuTest* tc, int expected, int actual);
void CuAssertPtrEquals(CuTest* tc, void* expected, void* actual);
void CuAssertPtrNotNull(CuTest* tc, void* pointer);
*/

void TestCurms(CuTest *tc)
{
        time_t time_in_s = time(NULL);
        curms_t time_in_ms = curms();
        /* There is a miniscule chance that this fails. */
        CuAssertIntEquals(tc, time_in_s, time_in_ms / 1000);
}

CuSuite *CuGetCltimeSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestCurms);

        return suite;
}
