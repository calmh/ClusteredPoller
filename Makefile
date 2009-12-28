# Main sources list
SOURCES = main.cpp query.cpp util.cpp globals.cpp snmp.cpp
TESTSOURCES = query.cpp util.cpp globals.cpp snmp-mock.cpp

# Find out which OS we are compiling under
OS := $(shell uname -s | awk '{print tolower($$0)}')

# Set LIBS and CXXFLAGS accordingly
ifeq ($(OS),linux)
	CXXFLAGS = -ansi -g -Wall -Werror -DOS_${OS} -O2 -DUSE_MYSQL -I/usr/include/mysql -I/usr/include/mysql++
	LIBS = -lnetsnmp -lpthread -lmysqlpp
else ifeq ($(OS),darwin)
	CXXFLAGS = -ansi -g -Wall -Werror -DOS_${OS}
	LIBS = -lnetsnmp
endif

OBJS := $(SOURCES)
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
TESTOBJS := $(TESTSOURCES)
TESTOBJS := $(TESTOBJS:.cpp=.o)
TESTOBJS := $(TESTOBJS:.c=.o)
TARGET := clpoll

all: $(TARGET) quicktest

$(TARGET): $(OBJS)
	g++ $^ $(LIBS) -o $@

test: ${TESTOBJS} tests.cpp
	make -C CppUnitLite
	g++ -DLONGTESTS $^ $(LIBS) CppUnitLite/cppunitlite.a -o testrunner
	./testrunner

quicktest: ${TESTOBJS} tests.cpp
	make -C CppUnitLite
	g++ $^ $(LIBS) CppUnitLite/cppunitlite.a -o quicktestrunner
	./quicktestrunner

clean:
	rm -f *.o ${TARGET} version.h testrunner quicktestrunner
	make -C CppUnitLite clean

# Version magic
main.o: version.h
version.h: ${SOURCES}
	echo \#define CLPOLL_VERSION \"`git describe --always`\" > version.h
