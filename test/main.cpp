#include "UnitTest++.h"
#include "TestReporterStdout.h"

int main(int argc, char*argv[])
{
        using namespace UnitTest;
        TestReporterStdout reporter;
        TestRunner runner(reporter);
        int errors = runner.RunTestsIf(Test::GetTestList(), "QuickTests", True(), 0);
        if (argc > 1)
                errors += runner.RunTestsIf(Test::GetTestList(), "LongTests", True(), 0);
        return errors;
}

