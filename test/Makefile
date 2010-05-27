TESTSOURCES = main-for-tests.o ../src/queryablehost.cpp ../src/util.cpp ../src/globals.cpp ../src/rtgconf.cpp ../src/rtgtargets.cpp ../src/multithread.cpp ../src/poller.cpp ../src/monitor.cpp snmp-mock.cpp database-mock.cpp tests.cpp
TESTOBJS := $(TESTSOURCES)
TESTOBJS := $(TESTOBJS:.cpp=.o)
TESTOBJS := $(TESTOBJS:.c=.o)
CXXFLAGS := $(CXXFLAGS) -DTESTSUITE

main-for-tests.o: ../src/main.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

test: ${TESTOBJS}
	g++ -DLONGTESTS $^ $(CXXFLAGS) $(LIBS) ../CppUnitLite/cppunitlite.a -o testrunner
	./testrunner

clean:
	rm -f *.o ${TARGET} testrunner

