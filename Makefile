INCLUDES = -I../include -I../CppUnitLite
OS = $(shell uname -s | awk '{print tolower($$0)}')
CXXFLAGS = -ansi -g -Wall -Werror -DOS_${OS} -DUSE_MYSQL $(INCLUDES)
ifeq ($(OS),linux)
	CXXFLAGS := $(CXXFLAGS) -I/usr/include/mysql -I/usr/include/mysql++
	LIBS = -lnetsnmp -lpthread -lmysqlp
else ifeq ($(OS),darwin)
	CXXFLAGS := $(CXXFLAGS) -I/usr/local/mysql/include -I/usr/local/include/mysql++
	LIBS = -lnetsnmp -lmysqlpp
endif
export CXXFLAGS
export LIBS

all:
	make -C CppUnitLite
	make -C src

clean:
	make -C CppUnitLite clean
	make -C src clean
	make -C test clean

.PHONY: test
test: all
	make -C test test

quicktest:
	make -C test quicktest

