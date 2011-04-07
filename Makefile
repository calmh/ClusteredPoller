OBJS = src/clbuf.o \
	src/clgstr.o \
	src/cllog.o \
	src/clsnmp.o \
	src/database.o \
	src/globals.o \
	src/main.o \
	src/monitor.o \
	src/multithread.o \
	src/poller.o \
	src/queryablehost.o \
	src/rtgconf.o \
	src/rtgtargets.o

TESTOBJS = src/clbuf.o \
	src/clgstr.o \
	src/cllog.o \
	src/globals.o \
	src/monitor.o \
	src/multithread.o \
	src/poller.o \
	src/queryablehost.o \
	src/rtgconf.o \
	src/rtgtargets.o \
	test/clbuftests.o \
	test/clsnmp-mock.o \
	test/cutest.o \
	test/database-mock.o \
	test/integrationtests.o \
	test/longtests.o \
	test/main.o \
	test/rtgconftests.o \
	test/rtgtargetstests.o \
	test/utiltests.o

TARGET := clpoll
TESTTARGET := testrunner
.SUFFIXES: .o .c

CFLAGS ?= -Wall

OS = $(shell uname -s | awk '{print tolower($$0)}')
ifeq ($(OS),linux)
	CFLAGS += -pthread $(shell mysql_config --include)
	LIBS = -lnetsnmp $(shell mysql_config --libs)
else ifeq ($(OS),darwin)
	CFLAGS += -I/usr/local/mysql/include
	LIBS = -L/usr/local/mysql/lib -lnetsnmp -lmysqlclient
endif

all: $(TARGET) quicktest

$(TARGET): CFLAGS += -O2
$(TARGET): $(OBJS)
	gcc $^ $(LIBS) -o $@
	strip $@

$(TARGET)-dbg: CFLAGS += -g
$(TARGET)-dbg: $(OBJS)
	gcc $^ $(LIBS) -o $@

$(TESTTARGET): CFLAGS += -Isrc
$(TESTTARGET): $(TESTOBJS)
	gcc $^ $(LIBS) -o $@

.PHONY: test
test: $(TESTTARGET)
	./$(TESTTARGET) long 2>/dev/null

quicktest: $(TESTTARGET)
	./$(TESTTARGET) 2>/dev/null

clean:
	rm -f $(OBJS) $(TESTOBJS) $(TARGET) $(TESTTARGET) $(TARGET)-dbg

.PHONY: reformat
reformat:
	indent src/*.c src/*.h test/*.c

.PHONY: version
version:
	echo "#define CLPOLL_VERSION \"${VERSION}\"" > src/version.h

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $(patsubst %.c, %.o, $<)

