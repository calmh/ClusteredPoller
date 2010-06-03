SOURCES = src/main.cpp \
	src/util.cpp \
	src/globals.cpp \
	src/snmp.cpp \
	src/queryablehost.cpp \
	src/rtgconf.cpp \
	src/rtgtargets.cpp \
	src/multithread.cpp \
	src/monitor.cpp \
	src/poller.cpp \
	src/database.cpp

TESTSOURCES = test/main.cpp \
	test/utiltests.cpp \
	test/integrationtests.cpp \
	test/rtgconftests.cpp \
	test/rtgtargetstests.cpp \
	test/longtests.cpp \
	test/database-mock.cpp \
	test/snmp-mock.cpp \
	src/util.cpp \
	src/globals.cpp \
	src/queryablehost.cpp \
	src/rtgconf.cpp \
	src/rtgtargets.cpp \
	src/multithread.cpp \
	src/monitor.cpp \
	src/poller.cpp \

OBJS := $(SOURCES)
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
TESTOBJS := $(TESTSOURCES)
TESTOBJS := $(TESTOBJS:.cpp=.o)
TESTOBJS := $(TESTOBJS:.c=.o)
TARGET := clpoll
TESTTARGET := testrunner
UNITTESTPP := UnitTest++/libUnitTest++.a
.SUFFIXES: .o .cpp

CXXFLAGS ?= -ansi -DOS_${OS} -DUSE_MYSQL 

OS = $(shell uname -s | awk '{print tolower($$0)}')
ifeq ($(OS),linux)
	CXXFLAGS += -I/usr/include/mysql -I/usr/include/mysql++
	LIBS = -lnetsnmp -lpthread -lmysqlpp
else ifeq ($(OS),darwin)
	CXXFLAGS += -I/usr/local/mysql/include -I/usr/local/include/mysql++
	LIBS = -lnetsnmp -lmysqlpp
endif

all: $(UNITTESTPP) $(TARGET) test

$(TARGET): CXXFLAGS += -O2 -Wall -Werror
$(TARGET): $(OBJS)
	@echo Linking $@...
	@g++ $^ $(LIBS) -o $@
	@strip $@

$(TESTTARGET): CXXFLAGS += -IUnitTest++/src -Isrc
$(TESTTARGET): $(TESTOBJS) $(UNITTESTPP)
	@echo Linking $@...
	@g++ $^ $(LIBS) $(UNITTESTPP) -o $@

.PHONY: test
test: $(TESTTARGET)
	@echo Running unit tests...
	@./$(TESTTARGET) 2>/dev/null

longtest: $(TESTTARGET)
	@echo Running unit tests...
	@./$(TESTTARGET) long 2>/dev/null

$(UNITTESTPP):
	@make -C UnitTest++

distclean: clean
	@make -C UnitTest++ clean

clean:
	@rm -f $(OBJS) $(TESTOBJS) $(TARGET) $(TESTTARGET)

.PHONY: reformat
reformat:
	@astyle -A8 -n --convert-tabs --align-pointer=type -z2 src/*.cpp src/*.h test/*.cpp

.PHONY: version
version:
	@echo "#define CLPOLL_VERSION \"${VERSION}\"" > include/version.h

%.o : %.cpp
	@echo $<
	@$(CXX) $(CXXFLAGS) -c $< -o $(patsubst %.cpp, %.o, $<)

