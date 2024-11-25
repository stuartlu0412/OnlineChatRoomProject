#define SERVER_PORT 4445
#define MAX_MSG_LEN 128
#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <memory>
#include <cstring>
#include <cstdio>
#include <fstream>

using Password = std::string;
using Username = std::string;
using Socket = int;
using IPString = std::string;
using Port = int;

// ----------------------- UserManager Class -----------------------

class UserManager {
private:

    std::string filename = "./UserDB"; // the filename of the db
    //FILE* UserDB;
    //std::ifstream UserDBread;
    //std::ostream UserDBwrite;

    bool userExist(const Username& username); // 
    Password hashPasswd(const Password& passwd);

public:

    UserManager() 
    {
        //UserDB = fopen("./UserDB", "a+");
    }
    
    int registerUser(const Username& username, const Password& passwd);
    int loginUser(const Username& username, const Password& passwd);
    int logoutUser(const Username&);

};

bool UserManager::userExist(const Username& username) {

    std::ifstream UserDB("./UserDB");
    //std::cout << "Checking user..." << std::endl;

    std::string line;
    while (std::getline(UserDB, line)) {
        size_t splitPos = line.find('|');
        if (splitPos != std::string::npos) {
            Username dbUsername = line.substr(0, splitPos);
            Password dbPassword = line.substr(splitPos + 1);

            if (dbUsername == username) {
                return true;
            }
        }
    }

    return false;
}

int UserManager::registerUser(const Username& username, const Password& passwd) {
    /*
        1: register success
        0: User already exist
    */
    if (userExist(username)) return 0;

    std::ofstream userDB("./UserDB", std::ios::app);

    userDB << username << '|' << passwd << '\n';

    return 1;

}

int UserManager::loginUser(const Username& username, const Password& passwd) {
    /* Return value:
        1 : login succuss
        0 : username does not exist
        -1: wrong password
    */
    // pass through use db to fetch user data
    std::ifstream UserDB("./UserDB");
    //std::cout << "Checking user..." << std::endl;

    std::string line;
    while (std::getline(UserDB, line)) {
        size_t splitPos = line.find('|');
        if (splitPos != std::string::npos) {
            Username dbUsername = line.substr(0, splitPos);
            Password dbPassword = line.substr(splitPos + 1);

            if (dbUsername == username) {
                return (dbPassword == passwd) ? 1 : -1; // Match or wrong password
            }
        }
    }

    return 0;
}

int UserManager::logoutUser(const Username&) {

}

// ----------------------- Client Class -------------------------------

/*
class Client { // is a class of connecting client
private:
    Socket clientSocket;
    struct sockaddr_in clientAddr;
    Username clientUserName;

    int readMessage();
    int handleCommand();
    int sendMessage();

public:
    Client (Socket clientSocket_)
        : clientSocket (clientSocket_)
    {
        std::cout << "New client initiated with socket = " << clientSocket << std::endl;
    } // constructor
};

*/

// ----------------------- Server Class -------------------------------

class Server {

private:

    Port serverPort;
    Socket serverSocket;
    Socket clientSocket;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    bool clientLoggedIn = false;

    void acceptConnections();

    int handleMessage(Socket clientSocket);
    std::string readFromClient(Socket clientSocket);
    int writeToClient(Socket clientSocket, std::string&& message);

    void clientDisconnect();


public:
    Server (Port port_);

    ~Server ()
    {
        close(serverSocket);
        //close(clientSocket);
        
        std::cout << "Server closed." << std::endl; 
    }

    void run();

};

Server::Server (Port port_)
    : serverPort (port_)
{  // Constructor
    //initialize server
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) ERR_EXIT("socket");

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(serverPort);

    if (bind(serverSocket, (struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr)) < 0) ERR_EXIT("bind");
    if (listen(serverSocket, 1024) < 0) ERR_EXIT("listen");

    std::cout << "Server created succesfully\n";

}

void Server::run() {
    // start listening
    std::cout << "Server start listening with port " << serverPort << std::endl;

    acceptConnections();
    handleMessage(clientSocket);
}

void Server::acceptConnections() {

    // initiate a client instance
    
    size_t client_len = sizeof(client_addr);
    clientSocket = accept(serverSocket, (struct sockaddr*) &client_addr, (socklen_t*) &client_len);
    char* clientIP_ptr = inet_ntoa(client_addr.sin_addr);
    std::cout << "Client address: " << clientIP_ptr << " connected with port number: " << client_addr.sin_port << ", fd = " << clientSocket << std::endl;
    // print out a connecting message to client
    
}

std::string Server::readFromClient(Socket clientSocket) {

    char buf[MAX_MSG_LEN];
    memset(buf, 0, MAX_MSG_LEN);
    int n;
    if ((n = read(clientSocket, buf, MAX_MSG_LEN)) < 0) perror("read from socket");

    std::cerr << n << std::endl;

    char* end = strstr(buf, "\r\n");
    if (end == NULL) {
        end = strchr(buf, '\n');
    }
    buf[end - buf] = '\0';
    std::string msg = std::string(buf);
    std::cerr << msg << ' ' << msg.size() << std::endl;

    return msg;
}

int Server::writeToClient(Socket clientSocket, std::string&& message) {

    if (write(clientSocket, message.c_str(), message.length()) < 0) {
        perror("write to socket");
        return -1;
    }
    
    return 1;
}

int Server::handleMessage(Socket clientSocket) {

    UserManager userManager;
    Username username;
    Password password;

    while (true) {
        // read in message

        writeToClient(clientSocket, "Please choose to Login, Logout or Register\n");

        std::string clientMessage = readFromClient(clientSocket);

        std::cerr << clientMessage << ' ' << clientMessage.size();

        // check message format
        if (clientMessage == "Login") {

            std::cout << "User demand to login" << "\n";

            if (clientLoggedIn) {
                writeToClient(clientSocket, "User is already logged in.\n");
                continue;
            }

            writeToClient(clientSocket, "Please enter Username: \n");

            username = readFromClient(clientSocket);

            writeToClient(clientSocket, "Please enter Password: \n");

            password = readFromClient(clientSocket);

            std::cout << "Client Username: " << username << std::endl << "Client Password: " << password << std::endl;

            int loginResult = userManager.loginUser(username, password);

            if (loginResult == 1) {
                // if login is successful
                clientLoggedIn = true;
                writeToClient(clientSocket, "Login Successfully!\n");

            } else if (loginResult == 0) {
                // user does not exist
                writeToClient(clientSocket, "User is not registered, please register\n");
                
            } else {
                // wrong password
                writeToClient(clientSocket, "Wrong password. Please try again\n");

            }

        } else if (clientMessage == "Register") {

            if (clientLoggedIn) {
                writeToClient(clientSocket, "User is already logged in.\n");
                continue;
            }

            writeToClient(clientSocket, "Please enter Username: \n");

            username = readFromClient(clientSocket);

            writeToClient(clientSocket, "Please enter Password: \n");

            password = readFromClient(clientSocket);

            std::cout << "Client Username: " << username << std::endl << "Client Password: " << password << std::endl;

            std::cout << "User demand to Register" << "\n";

            int registerResult = userManager.registerUser(username, password);

            if (registerResult) {
                writeToClient(clientSocket, "Register Successfully!\n");
            } else {
                writeToClient(clientSocket, "User already exist\n");
            }

        } else if (clientMessage == "Logout") {

            std::cout << "User demand to logout" << "\n";

            if (!clientLoggedIn) {
                writeToClient(clientSocket, "User is not logged in! Please Login\n");
            } else {
                clientLoggedIn = false;
                writeToClient(clientSocket, "User is logged out\n");
            }
        }
        // call Usermanager to handle message
        // print message to client
    }

}


int main(int argc, char* argv[]) {
    
    Port port;
    sscanf(argv[1], "%d", &port);

    Server server = Server(port);
    server.run();

}