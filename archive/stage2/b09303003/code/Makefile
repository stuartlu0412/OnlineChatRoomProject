CXX       := g++
CXXFLAGS  := -Wall -Wextra -std=c++20 -pthread -O2 -Iinclude
GTEST_LIBS := -lgtest -lgtest_main -lpthread

# Binaries
BIN := server
BIN2 := client
TEST_BIN  := test

# Source Files
SERVER_SRCS := src/ThreadPool.cpp src/Connection.cpp src/Server.cpp src/UserManager.cpp server.cpp
CLIENT_SRCS := src/Client.cpp client.cpp
TEST_SRCS   := src/ThreadPool.cpp src/Connection.cpp src/Server.cpp src/UserManager.cpp tests/ThreadPoolTest.cpp

# Object Files
SERVER_OBJS := $(SERVER_SRCS:.cpp=.o)
CLIENT_OBJS := $(CLIENT_SRCS:.cpp=.o)
TEST_OBJS   := $(TEST_SRCS:.cpp=.o)

# Targets
all: $(BIN) $(BIN2)

$(BIN): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN2): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_BIN): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test-run: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(TEST_OBJS) $(TEST_BIN) $(BIN) $(BIN2)

.PHONY: all clean
