OBJS = src/clbuf.o \
	src/clgstr.o \
	src/clinsert.o \
	src/cllog.o \
	src/clsnmp.o \
	src/database.o \
	src/globals.o \
	src/main.o \
	src/monitor.o \
	src/multithread.o \
	src/poller.o \
	src/rtgconf.o \
	src/rtgtargets.o \
	src/xmalloc.o

TESTOBJS = src/clbuf.o \
	src/clgstr.o \
	src/clinsert.o \
	src/cllog.o \
	src/globals.o \
	src/monitor.o \
	src/multithread.o \
	src/poller.o \
	src/rtgconf.o \
	src/rtgtargets.o \
	src/xmalloc.o \
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

include version.mk
GITVERSION:=$(shell git describe --always 2>/dev/null | sed 's/^v//')
ifdef GITVERSION
	VERSION = "$(GITVERSION)"
endif

# Net-SNMP uses C89 meaning of "extern inline", so we need -fgnu89-inline
CFLAGS ?= -DVERSION='$(VERSION)'

OS = $(shell uname -s)
ifeq ($(OS),Darwin)
	CFLAGS += -std=c99 -fgnu89-inline -I/usr/local/mysql/include
	LDFLAGS += -L/usr/local/mysql/lib -lnetsnmp -lmysqlclient
else ifeq ($(OS),SunOS)
	CC = gcc
	CFLAGS += -pthreads $(shell /usr/mysql/bin/mysql_config --include)
	LDFLAGS += -pthreads -lnetsnmp $(shell /usr/mysql/bin/mysql_config --libs | sed 's/-lCrun//' )
else ifeq ($(OS),Linux)
	CFLAGS += -std=c99 -fgnu89-inline -pthread -D_GNU_SOURCE -I/usr/local/include $(shell mysql_config --include)
	LDFLAGS += -pthread -L/usr/local/lib -lnetsnmp $(shell mysql_config --libs)
else ifeq ($(OS),FreeBSD)
	CFLAGS += -std=c99 -fgnu89-inline -pthread -I/usr/local/include $(shell mysql_config --include)
	LDFLAGS += -pthread -L/usr/local/lib -lnetsnmp $(shell mysql_config --libs)
endif

all: $(TARGET) quicktest

$(TARGET): CFLAGS += -O2
$(TARGET): $(OBJS)
	gcc $^ $(LDFLAGS) -o $@
	strip $@

debug: $(TARGET)-dbg quicktest
$(TARGET)-dbg: CFLAGS += -g -pedantic -O1 -Wall -Wextra -Werror -Wwrite-strings -Winit-self -Wcast-align -Wpointer-arith -Wformat=2 -Wmissing-declarations -Wold-style-definition -Wstrict-prototypes -Wmissing-prototypes -Wfloat-equal -Wswitch-default -Wswitch-enum -Wunused -Wshadow -Wcast-align
$(TARGET)-dbg: $(OBJS)
	gcc $^ $(LDFLAGS) -o $@

$(TESTTARGET): CFLAGS += -Isrc
$(TESTTARGET): $(TESTOBJS)
	gcc $^ $(LDFLAGS) -o $@

.PHONY: test
test: $(TESTTARGET)
	./$(TESTTARGET) long 2>/dev/null

.PHONY: quicktest
quicktest: $(TESTTARGET)
	./$(TESTTARGET) 2>/dev/null

.PHONY: clean
clean:
	rm -rf $(OBJS) $(TESTOBJS) $(TARGET) $(TESTTARGET) $(TARGET)-dbg doc

.PHONY: doc
doc:
	doxygen doxygen.conf

.PHONY: reformat
reformat:
	indent src/*.c src/*.h test/*.c
	rm */*~

.PHONY: version
version:
	echo "VERSION=\"${VERSION}\"" > version.mk

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $(patsubst %.c, %.o, $<)

