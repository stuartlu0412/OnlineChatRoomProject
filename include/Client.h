#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <netinet/in.h>
#include <pthread.h>
#include <atomic>

struct ChatSessionArgs {
    int socket;
};

/**
 * @brief A class to manage the client-side operations for server communication, 
 *        P2P connections, and chat functionality.
 */
class Client {
public:
    /**
     * @brief Constructor to initialize the client.
     * @param listenPort The port on which the client listens for peer connections.
     * @param serverIP Server's IP address.
     * @param serverPort Server's port number.
     */
    Client(uint16_t listenPort, const std::string& serverIP, uint16_t serverPort);

    /**
     * @brief Destructor to clean up resources.
     */
    ~Client();

    // Server Communication
    bool registerWithServer(const std::string& username, const std::string& password);
    bool loginToServer(const std::string& username, const std::string& password);
    bool logoutFromServer();
    std::pair<std::string, uint16_t> getClientInfo(const std::string& username);

    // P2P Communication
    bool connectToClient(const std::string& ipAddress, uint16_t port);
    bool chatWithClient(const std::string& username);
    static void* startChatSession(void* arg);

    

private:

    int serverSocket_;       // Communicate with the server
    int listenerSocket_;     // Listening socket for incoming peer connections
    sockaddr_in listenerAddr_;
    pthread_t listenerThread_;  // Listener thread handle
    std::atomic<bool> running_; // Control flag for the listener thread

    // Listener Management
    static void* listenerLoop(void* arg);
    static void* handlePeerConnection(void* arg);

    // Initialization Helpers
    bool initializeListener(uint16_t listenPort);
    bool connectToServer(const std::string& serverIP, uint16_t serverPort);
};

#endif // CLIENT_H
