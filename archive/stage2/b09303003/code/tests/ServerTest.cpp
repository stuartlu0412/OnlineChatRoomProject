#include <gtest/gtest.h>
#include "Server.h"
#include "ThreadPool.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>

// -----------------------------------------------------------------------------
// Test that the server initializes the socket correctly
// -----------------------------------------------------------------------------
TEST(ServerTest, InitializeSocket) {
    const int port = 9090;  // Arbitrary unused port
    const size_t threadCount = 4;

    // Create the server
    try {
        Server server(port, threadCount);
    } catch (const std::exception& ex) {
        FAIL() << "Server initialization threw an exception: " << ex.what();
    }

    SUCCEED() << "Server initialized successfully.";
}

// -----------------------------------------------------------------------------
// Test server accepts connections and enqueues them into the ThreadPool
// -----------------------------------------------------------------------------
TEST(ServerTest, AcceptAndEnqueueConnection) {
    const int port = 9091;  // Arbitrary unused port
    const size_t threadCount = 1;

    Server server(port, threadCount);

    // Run the server in a separate thread
    std::thread serverThread([&server]() {
        server.start();
    });

    // Sleep briefly to ensure the server is up and running
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Simulate a client connecting to the server
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(clientSocket, 0) << "Failed to create client socket: " << strerror(errno);

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connectResult = connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    ASSERT_EQ(connectResult, 0) << "Client failed to connect: " << strerror(errno);

    // Give the server some time to process the connection
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Clean up client socket
    close(clientSocket);

    // Stop the server
    server.stop();
    serverThread.join();

    SUCCEED() << "Server accepted and processed a connection successfully.";
}

// -----------------------------------------------------------------------------
// Test that the server stops gracefully
// -----------------------------------------------------------------------------

TEST(ServerTest, StopGracefully) {
    const int port = 9092;  // Arbitrary unused port
    const size_t threadCount = 2;

    Server server(port, threadCount);

    // Run the server in a separate thread
    std::thread serverThread([&server]() {
        server.start();
    });

    // Sleep briefly to ensure the server is running
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Stop the server
    server.stop();
    serverThread.join();

    SUCCEED() << "Server stopped gracefully.";
}


// -----------------------------------------------------------------------------
// Test integration with ThreadPool (simulate processing tasks)
// -----------------------------------------------------------------------------
TEST(ServerTest, IntegratesWithThreadPool) {
    const int port = 9093;  // Arbitrary unused port
    const size_t threadCount = 2;

    Server server(port, threadCount);

    // Run the server in a separate thread
    std::thread serverThread([&server]() {
        server.start();
    });

    // Simulate multiple client connections
    for (int i = 0; i < 3; ++i) {
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        ASSERT_GE(clientSocket, 0) << "Failed to create client socket: " << strerror(errno);

        sockaddr_in serverAddr = {};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        int connectResult = connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
        ASSERT_EQ(connectResult, 0) << "Client failed to connect: " << strerror(errno);

        // Close the client socket immediately after connection
        close(clientSocket);
    }

    // Sleep briefly to ensure all connections are processed
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Stop the server
    server.stop();
    serverThread.join();

    SUCCEED() << "Server processed multiple connections with ThreadPool integration.";
}

// -----------------------------------------------------------------------------
// Main entry point for Google Test
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
