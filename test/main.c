#include <stdio.h>
#include <string.h>
#include "cutest.h"

CuSuite *CuGetCBufferSuite();
CuSuite *CuGetIntegrationSuite();
CuSuite *CuGetLongSuite();
CuSuite *CuGetRtgConfSuite();
CuSuite *CuGetRtgTargetsSuite();
CuSuite *CuGetUtilSuite();
CuSuite *CuGetCltimeSuite();

void RunAllTests(int longtests)
{
        CuString *output = CuStringNew();
        CuSuite *suite = CuSuiteNew();

        CuSuiteAddSuite(suite, CuGetCBufferSuite());
        CuSuiteAddSuite(suite, CuGetIntegrationSuite());
        if (longtests)
                CuSuiteAddSuite(suite, CuGetLongSuite());
        CuSuiteAddSuite(suite, CuGetRtgConfSuite());
        CuSuiteAddSuite(suite, CuGetRtgTargetsSuite());
        CuSuiteAddSuite(suite, CuGetUtilSuite());
        CuSuiteAddSuite(suite, CuGetCltimeSuite());

        CuSuiteRun(suite);
        CuSuiteSummary(suite, output);
        CuSuiteDetails(suite, output);
        printf("%s\n", output->buffer);
}

int main(int argc, char **argv)
{
        if (argc > 1 && !strcmp(argv[1], "long"))
                RunAllTests(1);
        else
                RunAllTests(0);
        return 0;
}
