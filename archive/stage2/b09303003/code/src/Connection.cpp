#include "Connection.h"
#include "UserManager.h"

#include <iostream>    // For std::cerr, std::cout (debugging/logging)
#include <sstream>
#include <cstring>     // For strerror
#include <cerrno>      // For errno
#include <arpa/inet.h>

#ifdef USE_OPENSSL
#include <openssl/err.h> // For SSL error strings
#endif

// -----------------------------------------------------------------------------
// Constructor: Store the socket FD and client address, set connected_ = true.
// -----------------------------------------------------------------------------
Connection::Connection(int socketFd, const sockaddr_in& clientAddr, UserManager& userManager)
    : socketFd_(socketFd),
      clientAddr_(clientAddr),
      userManager_(userManager), 
      connected_(true)
#ifdef USE_OPENSSL
    , sslHandle_(nullptr)
#endif
{
    // Optionally, you can store or print the client's IP/port:
    // char clientIP[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, &clientAddr_.sin_addr, clientIP, INET_ADDRSTRLEN);
    // std::cout << "New Connection from " << clientIP
    //           << ":" << ntohs(clientAddr_.sin_port) << std::endl;
}

// -----------------------------------------------------------------------------
// Destructor: Ensure we close the connection if still open.
// -----------------------------------------------------------------------------
Connection::~Connection()
{
    closeConnection();
}

// -----------------------------------------------------------------------------
// handleClient(): The main loop for interacting with the client. 
// Reads data until client disconnects or an error occurs, 
// then processes the data as needed.
// -----------------------------------------------------------------------------
void Connection::handleClient()
{
    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    // Keep receiving data until an error or disconnect
    while (connected_) {
        // 1) Receive data
        ssize_t bytesRead = receiveData(buffer, BUFFER_SIZE);
        if (bytesRead <= 0) {
            // If 0 or negative, the client likely disconnected or an error occurred
            break;
        }

        // 2) Process the data (application-specific logic)
        processData(buffer, static_cast<size_t>(bytesRead));

        // Depending on your protocol, you might send a response via sendData() here
        // For example:
        //sendData("OK", 2);
    }

    // If we reach here, the loop ended => close the connection
    closeConnection();
}

// -----------------------------------------------------------------------------
// closeConnection(): Safely close the socket (and SSL if in use).
// -----------------------------------------------------------------------------
void Connection::closeConnection()
{
    if (!connected_) {
        return; // Already closed
    }

#ifdef USE_OPENSSL
    if (sslHandle_) {
        // Shutdown the SSL connection
        SSL_shutdown(sslHandle_);
        SSL_free(sslHandle_);
        sslHandle_ = nullptr;
    }
#endif

    if (socketFd_ >= 0) {
        ::close(socketFd_);
        socketFd_ = -1;
    }

    connected_ = false;
}

// -----------------------------------------------------------------------------
// Optional: Setup SSL for this connection (if USE_OPENSSL is defined).
// This method performs the SSL handshake.
// -----------------------------------------------------------------------------
#ifdef USE_OPENSSL
bool Connection::setupSSL(SSL_CTX* sslContext)
{
    if (!sslContext) {
        std::cerr << "SSL context is null.\n";
        return false;
    }

    sslHandle_ = SSL_new(sslContext);
    if (!sslHandle_) {
        std::cerr << "Failed to create SSL object.\n";
        return false;
    }

    // Bind the SSL object with our socket
    SSL_set_fd(sslHandle_, socketFd_);

    // Perform SSL handshake
    int ret = SSL_accept(sslHandle_);
    if (ret <= 0) {
        int error = SSL_get_error(sslHandle_, ret);
        std::cerr << "SSL_accept failed. Error: " << error << std::endl;
        ERR_print_errors_fp(stderr); // Print detailed error info
        SSL_free(sslHandle_);
        sslHandle_ = nullptr;
        return false;
    }

    std::cout << "SSL handshake successful.\n";
    return true;
}
#endif // USE_OPENSSL

// -----------------------------------------------------------------------------
// receiveData(): Read data from the socket or SSL.
// Returns the number of bytes read, or -1 on error, 0 if closed.
// -----------------------------------------------------------------------------
ssize_t Connection::receiveData(char* buffer, size_t size)
{
#ifdef USE_OPENSSL
    if (sslHandle_) {
        // Read via SSL
        int ret = SSL_read(sslHandle_, buffer, static_cast<int>(size));
        if (ret <= 0) {
            int error = SSL_get_error(sslHandle_, ret);
            if (error == SSL_ERROR_ZERO_RETURN) {
                // SSL connection closed
                return 0;
            } else {
                // Some other SSL read error
                std::cerr << "SSL_read error: " << error << std::endl;
                return -1;
            }
        }
        return ret;
    }
#endif

    // Non-SSL case: use recv()
    ssize_t bytesRead = ::recv(socketFd_, buffer, size, 0);
    if (bytesRead < 0) {
        std::cerr << "recv() failed: " << strerror(errno) << std::endl;
    }
    return bytesRead;
}

// -----------------------------------------------------------------------------
// sendData(): Write data to the socket or SSL.
// Returns the number of bytes written, or -1 on error.
// -----------------------------------------------------------------------------
ssize_t Connection::sendData(const char* data, size_t size)
{
#ifdef USE_OPENSSL
    if (sslHandle_) {
        int ret = SSL_write(sslHandle_, data, static_cast<int>(size));
        if (ret <= 0) {
            int error = SSL_get_error(sslHandle_, ret);
            std::cerr << "SSL_write error: " << error << std::endl;
            return -1;
        }
        return ret;
    }
#endif

    // Non-SSL case
    ssize_t bytesSent = ::send(socketFd_, data, size, 0);
    if (bytesSent < 0) {
        std::cerr << "send() failed: " << strerror(errno) << std::endl;
    }
    return bytesSent;
}

// -----------------------------------------------------------------------------
// processData(): A placeholder for application-specific logic.
// Here, we simply print the data. In a real server, you'd parse commands,
// handle user requests, etc.
// -----------------------------------------------------------------------------
void Connection::processData(const char* data, size_t size)
{
    // For demonstration, print out the received data
    std::string received(data, size);
    //std::cout << "[Connection] Received: " << received << std::endl;

    // In a chat server, you might parse commands here:
    // 1) Parse "REGISTER <user> <pass>"
    // 2) Parse "LOGIN <user> <pass>"
    // 3) Handle "LOGOUT", "SENDMESSAGE", etc.
    // 4) Possibly send responses via sendData(...).

    std::istringstream iss(received);
    std::string command;
    iss >> command;

    if (command == "REGISTER") {
        std::string username, password;
        iss >> username >> password;

        if (userManager_.registerUser(username, password)) {
            sendData("OK REGISTERED", 13);
        } else {
            sendData("ERR USER_EXISTS", 16);
        }
    } else if (command == "LOGIN") {
        std::string username, password, ip, port;
        iss >> username >> password >> ip >> port;

        std::string clientIP = inet_ntoa(clientAddr_.sin_addr);
        uint16_t clientPort = ntohs(clientAddr_.sin_port);

        if (userManager_.loginUser(username, password, clientIP, atoi(port.c_str()))) {
            sendData("OK LOGIN", 8);
        } else {
            sendData("ERR INVALID_CREDENTIALS", 24);
        }
    } else if (command == "LOGOUT") {
        std::string username;
        iss >> username;

        if (userManager_.logoutUser(username)) {
            sendData("OK LOGOUT", 9);
        } else {
            sendData("ERR NOT_LOGGED_IN", 17);
        }
    } else if (command == "GETINFO") {
        std::string targetUsername;
        iss >> targetUsername;

        User* user = userManager_.findUser(targetUsername);
        if (user && user->isLoggedIn) {
            std::string response = "OK " + user->ipAddress + ":" + std::to_string(user->port);
            sendData(response.c_str(), response.size());
        } else {
            sendData("ERR USER_NOT_FOUND", 18);
        }

    } else {
        sendData("ERR UNKNOWN_COMMAND", 20);
    }
}
