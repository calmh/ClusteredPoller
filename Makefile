# Main sources list
SOURCES = main.cpp query.cpp

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

objs := $(SOURCES:.cpp=.o)
target := clpoll

$(target): $(objs)
	g++ $^ $(LIBS) -o $@

clean:
	rm -f *.o ${target} version.h

# Extra dependencies
main.o: version.h types.h globals.h query.h
query.o: types.h globals.h query.h

# Version magic
version.h: ${SOURCES}
	echo \#define CLPOLL_VERSION \"`git describe --always`\" > version.h
