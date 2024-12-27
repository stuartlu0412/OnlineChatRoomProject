#include "Client.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sstream>


Client::Client(uint16_t listenPort, const std::string& serverIP, uint16_t serverPort) : running_(true) {
    // Initialize the listener
    if (!initializeListener(listenPort)) {
        throw std::runtime_error("Failed to initialize listener socket.");
    }

    // Connect to the server
    if (!connectToServer(serverIP, serverPort)) {
        throw std::runtime_error("Failed to connect to server.");
    }

    // Start the listener thread
    if (pthread_create(&listenerThread_, nullptr, &Client::listenerLoop, this) != 0) {
        throw std::runtime_error("Failed to create listener thread.");
    }

    std::cout << "Client initialized successfully." << std::endl;
}

Client::~Client() {
    running_ = false;

    // Close the listener socket
    close(listenerSocket_);

    // Join the listener thread
    if (pthread_join(listenerThread_, nullptr) != 0) {
        std::cerr << "Failed to join listener thread: " << strerror(errno) << std::endl;
    }

    // Close the server socket
    close(serverSocket_);

    std::cout << "Client resources cleaned up." << std::endl;
}

bool Client::initializeListener(uint16_t listenPort) {
    listenerSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenerSocket_ < 0) {
        std::cerr << "Failed to create listener socket: " << strerror(errno) << std::endl;
        return false;
    }

    listenerAddr_.sin_family = AF_INET;
    listenerAddr_.sin_addr.s_addr = INADDR_ANY;
    listenerAddr_.sin_port = htons(listenPort);

    if (bind(listenerSocket_, (sockaddr*)&listenerAddr_, sizeof(listenerAddr_)) < 0) {
        std::cerr << "Failed to bind listener socket: " << strerror(errno) << std::endl;
        close(listenerSocket_);
        return false;
    }

    if (listen(listenerSocket_, 5) < 0) {
        std::cerr << "Failed to listen on listener socket: " << strerror(errno) << std::endl;
        close(listenerSocket_);
        return false;
    }

    std::cout << "Listener initialized on port " << listenPort << std::endl;
    return true;
}

bool Client::connectToServer(const std::string& serverIP, uint16_t serverPort) {
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        std::cerr << "Failed to create server socket: " << strerror(errno) << std::endl;
        return false;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    if (connect(serverSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to connect to server: " << strerror(errno) << std::endl;
        close(serverSocket_);
        return false;
    }

    std::cout << "Connected to server at " << serverIP << ":" << serverPort << std::endl;
    return true;
}

bool Client::registerWithServer(const std::string& username, const std::string& password) {
    std::string command = "REGISTER " + username + " " + password + "\n";
    send(serverSocket_, command.c_str(), command.size(), 0);

    char buffer[1024];
    ssize_t bytesRead = recv(serverSocket_, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to register: " << strerror(errno) << std::endl;
        return false;
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer).substr(0, 2) == "OK";
}

bool Client::loginToServer(const std::string& username, const std::string& password) {
    char localIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &listenerAddr_.sin_addr, localIP, sizeof(localIP));

    std::string command = "LOGIN " + username + " " + password + " " + localIP + " " + std::to_string(ntohs(listenerAddr_.sin_port)) + "\n";
    send(serverSocket_, command.c_str(), command.size(), 0);

    char buffer[1024];
    ssize_t bytesRead = recv(serverSocket_, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to login: " << strerror(errno) << std::endl;
        return false;
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer).substr(0, 2) == "OK";
}

bool Client::logoutFromServer() {
    std::string command = "LOGOUT\n";
    send(serverSocket_, command.c_str(), command.size(), 0);

    char buffer[1024];
    ssize_t bytesRead = recv(serverSocket_, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to logout: " << strerror(errno) << std::endl;
        return false;
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer).substr(0, 2) == "OK";
}

std::pair<std::string, uint16_t> Client::getClientInfo(const std::string& username) {
    std::string command = "GETINFO " + username + "\n";
    send(serverSocket_, command.c_str(), command.size(), 0);

    char buffer[1024];
    ssize_t bytesRead = recv(serverSocket_, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        std::cerr << "Failed to get client info: " << strerror(errno) << std::endl;
        return {"", 0};
    }

    buffer[bytesRead] = '\0';
    std::string response(buffer);

    if (response.substr(0, 3) == "OK ") {
        size_t colonPos = response.find(':');
        std::string ip = response.substr(3, colonPos - 3);
        uint16_t port = static_cast<uint16_t>(std::stoi(response.substr(colonPos + 1)));
        return {ip, port};
    } else {
        std::cerr << "Error from server: " << response << std::endl;
        return {"", 0};
    }
}

bool Client::connectToClient(const std::string& ipAddress, uint16_t port) {
    int peerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (peerSocket < 0) {
        std::cerr << "Failed to create peer socket: " << strerror(errno) << std::endl;
        return false;
    }

    sockaddr_in peerAddr = {};
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &peerAddr.sin_addr);

    if (connect(peerSocket, (sockaddr*)&peerAddr, sizeof(peerAddr)) < 0) {
        std::cerr << "Failed to connect to client: " << strerror(errno) << std::endl;
        close(peerSocket);
        return false;
    }

    std::cout << "Connected to client at " << ipAddress << ":" << port << std::endl;
    close(peerSocket); // Chat can be implemented here
    return true;
}

void* Client::listenerLoop(void* arg) {
    Client* client = static_cast<Client*>(arg);

    while (client->running_) {
        sockaddr_in peerAddr;
        socklen_t peerLen = sizeof(peerAddr);
        int peerSocket = accept(client->listenerSocket_, (sockaddr*)&peerAddr, &peerLen);

        if (peerSocket < 0) {
            if (!client->running_) break; // Exit gracefully if stopping
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            continue;
        }

        std::cout << "Incoming chat request from " << inet_ntoa(peerAddr.sin_addr) << ":"
                  << ntohs(peerAddr.sin_port) << std::endl;

        // Start a chat session in a new thread
        auto* args = new ChatSessionArgs{peerSocket};
        pthread_t chatThread;
        if (pthread_create(&chatThread, nullptr, &Client::startChatSession, args) != 0) {
            std::cerr << "Failed to create thread for incoming chat." << std::endl;
            delete args;
            close(peerSocket);
        } else {
            pthread_detach(chatThread); // Detach thread to avoid join requirement
        }
    }

    return nullptr;
}


void* Client::handlePeerConnection(void* arg) {
    int peerSocket = *static_cast<int*>(arg);
    delete static_cast<int*>(arg); // Free the dynamically allocated memory

    char buffer[1024];
    while (true) {
        ssize_t bytesRead = recv(peerSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            std::cerr << "Peer connection closed." << std::endl;
            close(peerSocket);
            break;
        }

        buffer[bytesRead] = '\0';
        std::cout << "[Peer]: " << buffer << std::endl;
    }

    return nullptr;
}


/*
void* Client::startChatSession(void* arg) {
    auto* args = static_cast<ChatSessionArgs*>(arg);
    int socket = args->socket;
    delete args; // Clean up the dynamically allocated memory

    char buffer[1024];
    while (true) {
        ssize_t bytesRead = recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            std::cerr << "Connection closed by peer." << std::endl;
            break;
        }
        buffer[bytesRead] = '\0';
        std::cout << "[Peer]: " << buffer << std::endl;
    }

    close(socket);
    return nullptr;
}
*/
void* Client::startChatSession(void* arg) {
    auto* args = static_cast<ChatSessionArgs*>(arg);
    int socket = args->socket;
    delete args; // Clean up the dynamically allocated memory

    pthread_t sendThread, receiveThread;

    // Thread for sending messages
    auto sendMessages = [](void* arg) -> void* {
        int socket = *static_cast<int*>(arg);
        delete static_cast<int*>(arg); // Clean up dynamically allocated memory

        std::string message;
        while (true) {
            std::getline(std::cin, message);
            if (message == "quit") {
                close(socket);
                break;
            }
            send(socket, message.c_str(), message.size(), 0);
        }
        return nullptr;
    };

    // Thread for receiving messages
    auto receiveMessages = [](void* arg) -> void* {
        int socket = *static_cast<int*>(arg);
        delete static_cast<int*>(arg); // Clean up dynamically allocated memory

        char buffer[1024];
        while (true) {
            ssize_t bytesRead = recv(socket, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead <= 0) {
                std::cerr << "Connection closed by peer." << std::endl;
                close(socket);
                break;
            }
            buffer[bytesRead] = '\0';
            std::cout << "[Peer]: " << buffer << std::endl;
        }
        return nullptr;
    };

    // Create and start threads for send and receive
    auto* sendArg = new int(socket);
    auto* receiveArg = new int(socket);

    pthread_create(&sendThread, nullptr, sendMessages, sendArg);
    pthread_create(&receiveThread, nullptr, receiveMessages, receiveArg);

    // Wait for threads to finish
    pthread_join(sendThread, nullptr);
    pthread_join(receiveThread, nullptr);

    return nullptr;
}


bool Client::chatWithClient(const std::string& username) {
    // Get target user info from the server
    auto [ip, port] = getClientInfo(username);
    if (ip.empty() || port == 0) {
        std::cerr << "Failed to find user " << username << "." << std::endl;
        return false;
    }

    // Attempt to connect to the target user
    std::cout << "Attempting to connect to " << username << " at " << ip << ":" << port << "..." << std::endl;
    int peerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (peerSocket < 0) {
        std::cerr << "Failed to create peer socket: " << strerror(errno) << std::endl;
        return false;
    }

    sockaddr_in peerAddr = {};
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &peerAddr.sin_addr);

    if (connect(peerSocket, (sockaddr*)&peerAddr, sizeof(peerAddr)) < 0) {
        std::cerr << "Failed to connect to " << username << ": " << strerror(errno) << std::endl;
        close(peerSocket);
        return false;
    }

    std::cout << "Connected to " << username << ". Start chatting!" << std::endl;

    // Start chat session in a new thread
    auto* args = new ChatSessionArgs{peerSocket};
    pthread_t chatThread;
    if (pthread_create(&chatThread, nullptr, &Client::startChatSession, args) != 0) {
        std::cerr << "Failed to create chat thread." << std::endl;
        delete args;
        close(peerSocket);
        return false;
    }

    pthread_detach(chatThread); // Detach thread to avoid join requirement
    return true;
}
