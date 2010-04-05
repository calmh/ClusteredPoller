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
	rm -f *.o $(TARGET) version.h

# DO NOT DELETE

database.o: ../include/database.h ../include/multithread.h
database.o: ../include/globals.h ../include/types.h ../include/rtgconf.h
database.o: ../include/rtgtargets.h
globals.o: ../include/types.h ../include/rtgconf.h ../include/rtgtargets.h
main.o: ../include/util.h ../include/types.h ../include/globals.h
main.o: ../include/rtgconf.h ../include/rtgtargets.h ../include/monitor.h
main.o: ../include/multithread.h ../include/poller.h ../include/database.h
main.o: ../include/version.h
monitor.o: ../include/monitor.h ../include/multithread.h
monitor.o: ../include/queryablehost.h ../include/globals.h ../include/types.h
monitor.o: ../include/rtgconf.h ../include/rtgtargets.h
multithread.o: ../include/multithread.h
poller.o: ../include/globals.h ../include/types.h ../include/rtgconf.h
poller.o: ../include/rtgtargets.h ../include/poller.h
poller.o: ../include/multithread.h ../include/snmp.h
poller.o: ../include/queryablehost.h
queryablehost.o: ../include/rtgtargets.h ../include/types.h
queryablehost.o: ../include/queryablehost.h ../include/globals.h
queryablehost.o: ../include/rtgconf.h ../include/util.h ../include/snmp.h
rtgconf.o: ../include/rtgconf.h ../include/util.h ../include/types.h
rtgtargets.o: ../include/rtgtargets.h ../include/util.h ../include/types.h
rtgtargets.o: ../include/globals.h ../include/rtgconf.h
snmp.o: ../include/snmp.h
util.o: ../include/util.h ../include/types.h
