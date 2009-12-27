#SOURCES = Cpp/Deque/Deque.cpp Cpp/Deque/DequeTest.cpp Cpp/TestHarness/Test.cpp Cpp/TestHarness/TestRegistry.cpp Cpp/TestHarness/TestResult.cpp Cpp/TemplateDeque/DequeTest.cpp
SOURCES = Failure.cpp SimpleString.cpp Test.cpp TestRegistry.cpp TestResult.cpp


OBJS := $(SOURCES:.cpp=.o)
CXXFLAGS = -ansi -g -Wall -Werror

cppunitlite.a: ${OBJS}
	ar -r $@ $^

clean:
	rm -f cppunitlite.a ${OBJS}