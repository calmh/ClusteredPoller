#include "CppUnitLite/TestHarness.h"

int main()
{
    TestResult tr;
    TestRegistry::runAllTests(tr);

  return 0;
}

/*
TEST(Foo, bar)
{
  long i = 43;
  LONGS_EQUAL(42, i);
}
*/
