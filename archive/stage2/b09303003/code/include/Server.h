#ifndef SERVER_H
#define SERVER_H

#include "ThreadPool.h"
#include "Connection.h"
#include "UserManager.h"
#include <netinet/in.h>  // For sockaddr_in
#include <atomic>
#include <memory>        // For std::unique_ptr
#include <vector>        // For managing connected clients
#include <string>

/**
 * @brief A class to manage the server:
 *  - Accepting connections.
 *  - Handling incoming sockets using a ThreadPool.
 */
class Server
{
public:
    /**
     * @brief Constructs the server.
     *
     * @param port The port number on which the server listens.
     * @param threadCount The number of threads in the ThreadPool.
     */
    Server(int port, size_t threadCount);

    /**
     * @brief Destroys the server, cleaning up resources.
     */
    ~Server();

    /**
     * @brief Starts the server and enters the accept loop.
     */
    void start();

    /**
     * @brief Stops the server gracefully.
     */
    void stop();

private:
    /**
     * @brief Initializes the server socket.
     *
     * @param port The port to bind the server.
     * @return true if successful, false otherwise.
     */
    bool initializeSocket(int port);

    /**
     * @brief The main loop for accepting connections.
     *        Each accepted socket is pushed to the ThreadPool.
     */
    void acceptLoop();

    /**
     * @brief Safely shuts down and cleans up resources.
     */
    void cleanup();

private:
    int serverSocket_;                     // The listening socket
    sockaddr_in serverAddr_;               // Server address structure
    std::atomic_bool running_;             // Server running state
    std::unique_ptr<ThreadPool> threadPool_; // ThreadPool for handling client sockets
    UserManager userManager_; // Manage users
};

#endif // SERVER_H
