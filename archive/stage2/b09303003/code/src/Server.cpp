#include "Server.h"
#include <iostream>
#include <cstring>
#include <unistd.h> // For close()
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h> // For inet_ntoa()

Server::Server(int port, size_t threadCount)
    : serverSocket_(-1),
      running_(false),
      threadPool_(std::make_unique<ThreadPool>(threadCount))
{
    if (!initializeSocket(port)) {
        throw std::runtime_error("Failed to initialize the server socket.");
    }
}

Server::~Server()
{
    stop();
    cleanup();
}

bool Server::initializeSocket(int port)
{
    // Create the socket
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return false;
    }

    // Set socket options (reuse address)
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        return false;
    }

    // Bind the socket to the specified port
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    serverAddr_.sin_port = htons(port);

    if (bind(serverSocket_, (struct sockaddr*)&serverAddr_, sizeof(serverAddr_)) < 0) {
        std::cerr << "Socket bind failed: " << strerror(errno) << std::endl;
        return false;
    }

    // Start listening on the socket
    if (listen(serverSocket_, SOMAXCONN) < 0) {
        std::cerr << "Socket listen failed: " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "Server initialized and listening on port " << port << std::endl;
    return true;
}

void Server::start()
{
    running_ = true;
    std::cout << "Server started. Waiting for connections..." << std::endl;

    // Enter the main accept loop
    acceptLoop();
}

void Server::stop()
{
    if (running_) {
        running_ = false;

        // Close the listening socket to break the accept() call
        if (serverSocket_ >= 0) {
            close(serverSocket_);
            serverSocket_ = -1;
        }

        std::cout << "Stopping server..." << std::endl;
    }
}

void Server::acceptLoop()
{
    while (running_) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        // Accept a new connection
        int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (running_) {
                std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            }
            continue;
        }

        // Log the incoming connection
        std::cout << "Accepted connection from "
                  << inet_ntoa(clientAddr.sin_addr) << ":"
                  << ntohs(clientAddr.sin_port) << std::endl;

        // Enqueue the client socket to the ThreadPool for processing
        //threadPool_->enqueue(clientSocket);

        // Enqueue the task into the ThreadPool
        threadPool_->enqueue([this, clientSocket, clientAddr]() {
            Connection connection(clientSocket, clientAddr, userManager_);
            connection.handleClient();
        });
    }
}

void Server::cleanup()
{
    // Close the listening socket
    if (serverSocket_ >= 0) {
        close(serverSocket_);
        serverSocket_ = -1;
    }

    std::cout << "Server resources cleaned up." << std::endl;
}
