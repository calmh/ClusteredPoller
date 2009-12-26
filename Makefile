# Main sources list
SOURCES = main.cpp query.cpp util.cpp globals.cpp
TESTSOURCES = tests.cpp query.cpp util.cpp globals.cpp

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

all: $(target) test

objs := $(SOURCES:.cpp=.o)
testobjs := $(TESTSOURCES:.cpp=.o)
target := clpoll

$(target): $(objs)
	g++ $^ $(LIBS) -o $@

test: ${testobjs}
	g++ $^ $(LIBS) CppUnitLite/cppunitlite.a -o testrunner
	./testrunner

clean:
	rm -f *.o ${target} version.h testrunner

# Extra dependencies
main.o: version.h types.h globals.h query.h
query.o: types.h globals.h query.h

# Version magic
version.h: ${SOURCES}
	echo \#define CLPOLL_VERSION \"`git describe --always`\" > version.h
