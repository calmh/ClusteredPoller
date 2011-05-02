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

void TestNextInterval(CuTest *tc)
{
        curms_t now = 13043516141234LL;
        unsigned polling_interval = 300;
        curms_t ni = next_interval(now, polling_interval);

        CuAssertIntEquals(tc, 13043516400000LL, ni);
}

CuSuite *CuGetCltimeSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestCurms);
        SUITE_ADD_TEST(suite, TestNextInterval);

        return suite;
}
