#include <gtest/gtest.h>

// Standard headers
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>   // for sockaddr_in
#include <errno.h>

// Your project headers
#include "ThreadPool.h"
#include "Connection.h"

// -----------------------------------------------------------------------------
// 1) ThreadPool Tests
// -----------------------------------------------------------------------------

// Simple test to ensure the ThreadPool can be constructed and destroyed
TEST(ThreadPoolTest, ConstructAndDestruct)
{
    // Constructing the pool spawns threads; destroying it joins threads
    {
        ThreadPool pool(4);
        // If this block finishes without crashing or deadlocking, the test passes
    }
    SUCCEED() << "ThreadPool constructed and destructed successfully.";
}

// A bit more advanced test that tries enqueuing dummy sockets
TEST(ThreadPoolTest, EnqueueDummySockets)
{
    ThreadPool pool(2);

    // We'll enqueue a few dummy sockets. 
    // Because the worker loop calls `Connection connection(sock, {}); connection.handleClient();`
    // we won't see an obvious "result" unless we put print statements in handleClient(),
    // or we do something more elaborate (like a mock).
    for (int i = 0; i < 5; i++) {
        // -1 is typically invalid, but let's just put them in to see that
        // the code runs handleClient() anyway. In a real test, you might
        // pass real sockets or use a mocking approach.
        pool.enqueue(-1);
    }

    // Give the pool time to process
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // If it didn’t crash or deadlock, success. 
    // For more robust testing, we'd need a way to track that tasks actually ran.
    SUCCEED() << "Enqueued dummy sockets without crashing.";
}

// A test that ensures the ThreadPool stops cleanly
TEST(ThreadPoolTest, StopCleanly)
{
    // We'll create a pool, enqueue some sockets, then let the destructor run.
    {
        ThreadPool pool(2);
        for (int i = 0; i < 3; i++) {
            pool.enqueue(-1);
        }
        // Exiting this scope calls the destructor => sets stop_ = true => joins threads
    }
    SUCCEED() << "ThreadPool stopped cleanly.";
}

// -----------------------------------------------------------------------------
// 2) Connection Tests (using socketpair)
// -----------------------------------------------------------------------------

// Helper function: read from one side of a socketpair to gather data
static std::string readAll(int fd, int maxBytes = 1024)
{
    char buffer[1024];
    int bytesRead = ::recv(fd, buffer, maxBytes, 0);
    if (bytesRead > 0) {
        return std::string(buffer, bytesRead);
    }
    return {};
}

// Test that Connection can read data from a client (via socketpair)
TEST(ConnectionTest, BasicSocketPair)
{
    // Create a socketpair for local, in-process client/server
    int fds[2];
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0) 
        << "socketpair failed: " << strerror(errno);

    int clientFd = fds[0];
    int serverFd = fds[1];

    // Write a message from client side
    const char* msg = "Hello from the client!";
    ::send(clientFd, msg, strlen(msg), 0);

    // Create a Connection to handle the "serverFd"
    Connection conn(serverFd, {} /* no sockaddr needed for test */);

    // In a real server, handleClient() typically runs in a loop until disconnected.
    // This can block if handleClient() expects further input. 
    // We'll run it in a separate thread for safety.
    std::thread t([&conn]() {
        conn.handleClient(); 
        // By default, it may keep reading until 0 bytes or error -> once we close clientFd
        // handleClient should eventually exit.
    });

    // Give handleClient() time to do something
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Optionally send more data or close the client side
    ::close(clientFd);

    // Wait for handleClient() to return
    t.join();

    // Now the Connection destructor closes serverFd as well
    // If we want to see exactly what handleClient() did, we'd check logs or store state.

    SUCCEED() << "Connection handled socketpair without crashing.";
}

// -----------------------------------------------------------------------------
// 3) Simple Integration Test
// -----------------------------------------------------------------------------

// This test simulates a mini flow:
//  - Start a ThreadPool
//  - Use a socketpair for a "client" -> "server" connection
//  - Enqueue that server FD into the pool
//  - Then write from the client side, close it, and ensure everything shuts down.
TEST(IntegrationTest, ThreadPoolConnectionInteraction)
{
    ThreadPool pool(2);

    // 1) Create a socketpair 
    int fds[2];
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0)
        << "socketpair failed: " << strerror(errno);

    int clientFd = fds[0];
    int serverFd = fds[1];

    // 2) Enqueue serverFd in the pool -> workerLoop -> Connection(serverFd).handleClient()
    pool.enqueue(serverFd);

    // 3) Write from client side 
    const char* msg = "Hello from the client (integration)!";
    ::send(clientFd, msg, strlen(msg), 0);

    // 4) Close client side so handleClient() eventually sees EOF and returns
    ::close(clientFd);

    // 5) Sleep briefly to let the worker handle it
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 6) Destroy the pool -> signals stop_, joins threads
    // Done automatically by leaving scope

    SUCCEED() << "Integration test completed without deadlock or crash.";
}

// -----------------------------------------------------------------------------
// 4) Main Entry (if building a standalone test executable)
// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
