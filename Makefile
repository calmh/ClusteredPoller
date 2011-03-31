OBJS = src/main.o \
	src/util.o \
	src/globals.o \
	src/snmp.o \
	src/queryablehost.o \
	src/rtgconf.o \
	src/rtgtargets.o \
	src/multithread.o \
	src/monitor.o \
	src/poller.o \
	src/database.o

TESTOBJS = test/main.o \
	test/utiltests.o \
	test/integrationtests.o \
	test/rtgconftests.o \
	test/rtgtargetstests.o \
	test/longtests.o \
	test/database-mock.o \
	test/snmp-mock.o \
	src/util.o \
	src/globals.o \
	src/queryablehost.o \
	src/rtgconf.o \
	src/rtgtargets.o \
	src/multithread.o \
	src/monitor.o \
	src/poller.o \

TARGET := clpoll
TESTTARGET := testrunner
UNITTESTPP := UnitTest++/libUnitTest++.a
.SUFFIXES: .o .cpp .c

CXXFLAGS ?= -ansi -DOS_${OS} -DUSE_MYSQL -Wall -Werror
CFLAGS ?= --std=c99 -DOS_${OS} -DUSE_MYSQL -Wall

OS = $(shell uname -s | awk '{print tolower($$0)}')
ifeq ($(OS),linux)
	CXXFLAGS += -I/usr/include/mysql -I/usr/include/mysql++
	CFLAGS += -I/usr/include/mysql
	LIBS = -lnetsnmp -lpthread -lmysqlpp
else ifeq ($(OS),darwin)
	CXXFLAGS += -I/usr/local/mysql/include -I/usr/local/include/mysql++
	CFLAGS += -I/usr/local/mysql/include
	LIBS = -lnetsnmp -lmysqlpp
endif

all: $(UNITTESTPP) $(TARGET) test

$(TARGET): CXXFLAGS += -O2
$(TARGET): CFLAGS += -O2
$(TARGET): $(OBJS)
	g++ $^ $(LIBS) -o $@
	strip $@

$(TESTTARGET): CXXFLAGS += -IUnitTest++/src -Isrc
$(TESTTARGET): CFLAGS += -IUnitTest++/src -Isrc
$(TESTTARGET): $(TESTOBJS) $(UNITTESTPP)
	g++ $^ $(LIBS) $(UNITTESTPP) -o $@

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
	rm -f $(OBJS) $(TESTOBJS) $(TARGET) $(TESTTARGET)

.PHONY: reformat
reformat:
	astyle -A8 -n --convert-tabs --align-pointer=type -z2 src/*.cpp src/*.h test/*.cpp

.PHONY: version
version:
	echo "#define CLPOLL_VERSION \"${VERSION}\"" > src/version.h

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $(patsubst %.cpp, %.o, $<)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $(patsubst %.c, %.o, $<)

