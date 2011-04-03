#include <string.h>
#include "clbuf.h"
#include "cutest.h"

/*
void CuAssert(CuTest* tc, char* message, int condition);
void CuAssertTrue(CuTest* tc, int condition);
void CuAssertStrEquals(CuTest* tc, char* expected, char* actual);
void CuAssertIntEquals(CuTest* tc, int expected, int actual);
void CuAssertPtrEquals(CuTest* tc, void* expected, void* actual);
void CuAssertPtrNotNull(CuTest* tc, void* pointer);
*/

void TestEmptyBufferSize(CuTest *tc)
{
        struct clbuf *cb = clbuf_create(16);
        CuAssertIntEquals(tc,  0, clbuf_count_used(cb));
        CuAssertIntEquals(tc, 16, clbuf_count_free(cb));
}

void TestEmptyBufferPop(CuTest *tc)
{
        struct clbuf *cb = clbuf_create(16);
        CuAssertPtrEquals(tc, NULL, clbuf_pop(cb));
}

void TestBufferTwoItems(CuTest *tc)
{
        struct clbuf *cb = clbuf_create(16);
        clbuf_push(cb, "Test1");
        clbuf_push(cb, "Test2");
        CuAssertIntEquals(tc, 2, clbuf_count_used(cb));
        CuAssertStrEquals(tc, "Test1", clbuf_pop(cb));
        CuAssertStrEquals(tc, "Test2", clbuf_pop(cb));
        CuAssertIntEquals(tc, 0, clbuf_count_used(cb));
        CuAssertPtrEquals(tc, NULL, clbuf_pop(cb));
}

void TestBufferManyItems(CuTest *tc)
{
        struct clbuf *cb = clbuf_create(4);
        clbuf_push(cb, "Test1");
        clbuf_push(cb, "Test2");
        clbuf_push(cb, "Test3");
        CuAssertIntEquals(tc, 3, clbuf_count_used(cb));
        CuAssertStrEquals(tc, "Test1", clbuf_pop(cb));
        CuAssertStrEquals(tc, "Test2", clbuf_pop(cb));
        CuAssertIntEquals(tc, 1, clbuf_count_used(cb));
        CuAssertIntEquals(tc, 3, clbuf_count_free(cb));
        clbuf_push(cb, "Test4");
        clbuf_push(cb, "Test5");
        clbuf_push(cb, "Test6");
        CuAssertIntEquals(tc, 4, clbuf_count_used(cb));
        CuAssertIntEquals(tc, 0, clbuf_count_free(cb));
        CuAssertStrEquals(tc, "Test3", clbuf_pop(cb));
        CuAssertStrEquals(tc, "Test4", clbuf_pop(cb));
        CuAssertStrEquals(tc, "Test5", clbuf_pop(cb));
        CuAssertStrEquals(tc, "Test6", clbuf_pop(cb));
        CuAssertPtrEquals(tc, NULL, clbuf_pop(cb));
}

CuSuite *CuGetCBufferSuite(void)
{
        CuSuite *suite = CuSuiteNew();

        SUITE_ADD_TEST(suite, TestEmptyBufferSize);
        SUITE_ADD_TEST(suite, TestEmptyBufferPop);
        SUITE_ADD_TEST(suite, TestBufferTwoItems);
        SUITE_ADD_TEST(suite, TestBufferManyItems);

        return suite;
}

