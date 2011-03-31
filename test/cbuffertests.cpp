#include <string>

#include "UnitTest++.h"
#include "cbuffer.h"

using namespace std;

SUITE(QuickTests)
{
        TEST(EmptyBufferSize) {
                cbuffer* cb = cbuffer_create(16);
                CHECK_EQUAL(0u, cbuffer_count(cb));
                CHECK_EQUAL(16u, cbuffer_free(cb));
        }

        TEST(EmptyBufferPop) {
                cbuffer* cb = cbuffer_create(16);
                CHECK_EQUAL((void*)NULL, cbuffer_pop(cb));
        }

        TEST(BufferTwoItems) {
                cbuffer* cb = cbuffer_create(16);
                cbuffer_push(cb, (void*)"Test1");
                cbuffer_push(cb, (void*)"Test2");
                CHECK_EQUAL(2u, cbuffer_count(cb));
                CHECK_EQUAL((void*)"Test1", cbuffer_pop(cb));
                CHECK_EQUAL((void*)"Test2", cbuffer_pop(cb));
                CHECK_EQUAL(0u, cbuffer_count(cb));
                CHECK_EQUAL((void*)NULL, cbuffer_pop(cb));
        }

        TEST(BufferManyItems) {
                cbuffer* cb = cbuffer_create(4);
                cbuffer_push(cb, (void*)"Test1");
                cbuffer_push(cb, (void*)"Test2");
                cbuffer_push(cb, (void*)"Test3");
                CHECK_EQUAL(3u, cbuffer_count(cb));
                CHECK_EQUAL((void*)"Test1", cbuffer_pop(cb));
                CHECK_EQUAL((void*)"Test2", cbuffer_pop(cb));
                CHECK_EQUAL(1u, cbuffer_count(cb));
                CHECK_EQUAL(3u, cbuffer_free(cb));
                cbuffer_push(cb, (void*)"Test4");
                cbuffer_push(cb, (void*)"Test5");
                cbuffer_push(cb, (void*)"Test6");
                CHECK_EQUAL(4u, cbuffer_count(cb));
                CHECK_EQUAL(0u, cbuffer_free(cb));
                CHECK_EQUAL((void*)"Test3", cbuffer_pop(cb));
                CHECK_EQUAL((void*)"Test4", cbuffer_pop(cb));
                CHECK_EQUAL((void*)"Test5", cbuffer_pop(cb));
                CHECK_EQUAL((void*)"Test6", cbuffer_pop(cb));
                CHECK_EQUAL((void*)NULL, cbuffer_pop(cb));
        }
}
