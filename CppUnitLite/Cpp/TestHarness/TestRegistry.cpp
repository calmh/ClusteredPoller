

#include "Test.h"
#include "TestRegistry.h"
#include "TestResult.h"


void TestRegistry::addTest(Test *test)
{
	instance().add(test);
}


void TestRegistry::runAllTests(TestResult& result)
{
	instance().run(result);
}


TestRegistry& TestRegistry::instance()
{
	static TestRegistry registry;

	return registry;
}


void TestRegistry::add(Test *test)
{
	tests.push_back(test);
}


void TestRegistry::run(TestResult& result)
{
	result.startTests();
	for (std::vector<Test*>::iterator it = tests.begin(); it != tests.end(); ++it)
		(*it)->run(result);
	result.endTests();
}



