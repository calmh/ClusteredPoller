OBJS = src/cbuffer.o \
	src/database.o \
	src/globals.o \
	src/gstring.o \
	src/main.o \
	src/monitor.o \
	src/multithread.o \
	src/poller.o \
	src/queryablehost.o \
	src/rtgconf.o \
	src/rtgtargets.o \
	src/snmp.o \
	src/util.o 

TESTOBJS = src/cbuffer.o \
	src/globals.o \
	src/gstring.o \
	src/monitor.o \
	src/multithread.o \
	src/poller.o \
	src/queryablehost.o \
	src/rtgconf.o \
	src/rtgtargets.o \
	src/util.o \
	test/cbuffertests.o \
	test/cutest.o \
	test/database-mock.o \
	test/integrationtests.o \
	test/longtests.o \
	test/main.o \
	test/rtgconftests.o \
	test/rtgtargetstests.o \
	test/snmp-mock.o \
	test/utiltests.o 

TARGET := clpoll
TESTTARGET := testrunner
.SUFFIXES: .o .c

CFLAGS ?= -DOS_${OS} -Wall

OS = $(shell uname -s | awk '{print tolower($$0)}')
ifeq ($(OS),linux)
	CFLAGS += -I/usr/include/mysql
	LIBS = -lnetsnmp -lpthread -lmysqlclient
else ifeq ($(OS),darwin)
	CFLAGS += -I/usr/local/mysql/include
	LIBS = -L/usr/local/mysql/lib -lnetsnmp -lmysqlclient
endif

all: $(UNITTESTPP) $(TARGET) test

$(TARGET): CFLAGS += -O2
$(TARGET): $(OBJS)
	gcc $^ $(LIBS) -o $@
	strip $@

$(TARGET)-dbg: CFLAGS += -g
$(TARGET)-dbg: $(OBJS)
	gcc $^ $(LIBS) -o $@
	strip $@

$(TESTTARGET): CFLAGS += -Isrc
$(TESTTARGET): $(TESTOBJS) $(UNITTESTPP)
	gcc $^ $(LIBS) $(UNITTESTPP) -o $@

.PHONY: test
test: $(TESTTARGET)
	./$(TESTTARGET) 2>/dev/null

longtest: $(TESTTARGET)
	./$(TESTTARGET) long 2>/dev/null

$(UNITTESTPP):
	make -C UnitTest++

distclean: clean
	make -C UnitTest++ clean

clean:
	rm -f $(OBJS) $(TESTOBJS) $(TARGET) $(TESTTARGET) $(TARGET)-dbg

.PHONY: reformat
reformat:
	astyle -A8 -n --convert-tabs --align-pointer=name -z2 src/*.c src/*.h test/*.c

.PHONY: version
version:
	echo "#define CLPOLL_VERSION \"${VERSION}\"" > src/version.h

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $(patsubst %.c, %.o, $<)

