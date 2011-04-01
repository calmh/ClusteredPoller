#include <string.h>
#include "cbuffer.h"
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
        cbuffer *cb = cbuffer_create(16);
        CuAssertIntEquals(tc,  0, cbuffer_count(cb));
        CuAssertIntEquals(tc, 16, cbuffer_free(cb));
}

void TestEmptyBufferPop(CuTest *tc)
{
        cbuffer *cb = cbuffer_create(16);
        CuAssertPtrEquals(tc, NULL, cbuffer_pop(cb));
}

void TestBufferTwoItems(CuTest *tc)
{
        cbuffer *cb = cbuffer_create(16);
        cbuffer_push(cb, "Test1");
        cbuffer_push(cb, "Test2");
        CuAssertIntEquals(tc, 2, cbuffer_count(cb));
        CuAssertStrEquals(tc, "Test1", cbuffer_pop(cb));
        CuAssertStrEquals(tc, "Test2", cbuffer_pop(cb));
        CuAssertIntEquals(tc, 0, cbuffer_count(cb));
        CuAssertPtrEquals(tc, NULL, cbuffer_pop(cb));
}

void TestBufferManyItems(CuTest *tc)
{
        cbuffer *cb = cbuffer_create(4);
        cbuffer_push(cb, "Test1");
        cbuffer_push(cb, "Test2");
        cbuffer_push(cb, "Test3");
        CuAssertIntEquals(tc, 3, cbuffer_count(cb));
        CuAssertStrEquals(tc, "Test1", cbuffer_pop(cb));
        CuAssertStrEquals(tc, "Test2", cbuffer_pop(cb));
        CuAssertIntEquals(tc, 1, cbuffer_count(cb));
        CuAssertIntEquals(tc, 3, cbuffer_free(cb));
        cbuffer_push(cb, "Test4");
        cbuffer_push(cb, "Test5");
        cbuffer_push(cb, "Test6");
        CuAssertIntEquals(tc, 4, cbuffer_count(cb));
        CuAssertIntEquals(tc, 0, cbuffer_free(cb));
        CuAssertStrEquals(tc, "Test3", cbuffer_pop(cb));
        CuAssertStrEquals(tc, "Test4", cbuffer_pop(cb));
        CuAssertStrEquals(tc, "Test5", cbuffer_pop(cb));
        CuAssertStrEquals(tc, "Test6", cbuffer_pop(cb));
        CuAssertPtrEquals(tc, NULL, cbuffer_pop(cb));
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

