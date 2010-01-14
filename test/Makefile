# Main sources list
TESTSOURCES = ../src/query.cpp ../src/util.cpp ../src/globals.cpp snmp-mock.cpp

# Find out which OS we are compiling under
OS := $(shell uname -s | awk '{print tolower($$0)}')

# Set LIBS and CXXFLAGS accordingly
ifeq ($(OS),linux)
	CXXFLAGS = -ansi -g -Wall -Werror -DOS_${OS} -O2 -DUSE_MYSQL -I../include -I../CppUnitLite -I/usr/include/mysql -I/usr/include/mysql++
	LIBS = -lnetsnmp -lpthread -lmysqlpp
else ifeq ($(OS),darwin)
	CXXFLAGS = -ansi -g -Wall -Werror -DOS_${OS} -DUSE_MYSQL -I../include -I/usr/local/mysql/include -I/usr/local/include/mysql++
	LIBS = -lnetsnmp -lmysqlpp
endif

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

