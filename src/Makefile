SOURCES = main.cpp util.cpp globals.cpp snmp.cpp queryablehost.cpp rtgconf.cpp rtgtargets.cpp multithread.cpp monitor.cpp poller.cpp database.cpp
OBJS := $(SOURCES)
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
TESTOBJS := $(TESTSOURCES)
TESTOBJS := $(TESTOBJS:.cpp=.o)
TESTOBJS := $(TESTOBJS:.c=.o)
TARGET := clpoll

all: $(TARGET)

$(TARGET): $(OBJS)
	g++ $^ $(LIBS) -o $@

clean:
	rm -f *.o ${TARGET} version.h

# Version magic
main.o: version.h
version.h: ${SOURCES}
	echo \#define CLPOLL_VERSION \"`git describe --always`\" > version.h
