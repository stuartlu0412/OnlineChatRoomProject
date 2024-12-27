CXX       := g++
CXXFLAGS  := -Wall -Wextra -std=c++17 -pthread -O2 -Iinclude
GTEST_LIBS := -lgtest -lgtest_main -lpthread

BIN := main

TEST_BIN  := test

SRCS     := src/ThreadPool.cpp src/Connection.cpp src/Server.cpp
TEST_SRCS := tests/ServerTest.cpp  # or tests/Tests.cpp, etc.

OBJS     := $(SRCS:.cpp=.o)
TEST_OBJS := $(TEST_SRCS:.cpp=.o)

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST_LIBS)

$(TEST_BIN): $(OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test-run: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TEST_BIN) $(BIN)

.PHONY: all clean
