TESTSOURCES = ../src/queryablehost.cpp ../src/util.cpp ../src/globals.cpp ../src/rtgconf.cpp ../src/rtgtargets.cpp  snmp-mock.cpp
TESTOBJS := $(TESTSOURCES)
TESTOBJS := $(TESTOBJS:.cpp=.o)
TESTOBJS := $(TESTOBJS:.c=.o)

test: ${TESTOBJS} tests.cpp
	g++ -DLONGTESTS $^ $(CXXFLAGS) $(LIBS) ../CppUnitLite/cppunitlite.a -o testrunner
	./testrunner

quicktest: ${TESTOBJS} tests.cpp
	g++ $^ $(CXXFLAGS) $(LIBS) ../CppUnitLite/cppunitlite.a -o quicktestrunner
	./quicktestrunner

clean:
	rm -f *.o ${TARGET} testrunner quicktestrunner

