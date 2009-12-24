SOURCES = main.cpp

CXXFLAGS = -ansi -g -Wall -Werror -DOS_${OS} -O2 -I/usr/include/mysql -I/usr/include/mysql++
LINUX_LIBS = -lnetsnmp -lpthread -lmysqlpp
DARWIN_LIBS = -lnetsnmp

objs := $(SOURCES:.cpp=.o)
OS := $(shell uname -s | awk '{print toupper($$0)}')
os := $(shell uname -s | awk '{print tolower($$0)}')
arch := $(shell arch)
platformtarget := betterpoller.$(os)-$(arch)

all: $(platformtarget)

$(platformtarget): $(objs)
	g++ $^ $($(OS)_LIBS) -o $@

clean:
	rm -f *.o ${platformtarget}



