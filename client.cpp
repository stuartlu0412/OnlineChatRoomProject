#define MAX_MSG_LEN 128
#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring> // For memset

using Socket = int;

int writeToServer(Socket serverSocket, std::string& message) {

    if (write(serverSocket, message.c_str(), message.length()) < 0) {
        perror("write to socket");
        return -1;
    }
    //fprintf(stderr, "Write message %s to server", message.c_str());
    return 1;
}

std::string readFromServer(Socket clientSocket) {
    char buf[MAX_MSG_LEN];
    memset(buf, 0, MAX_MSG_LEN);
    if ((read(clientSocket, buf, MAX_MSG_LEN)) < 0) perror("read from socket");

    char* end = strstr(buf, "\r\n");
    if (end == NULL) {
        end = strchr(buf, '\n');
    }
    buf[end - buf] = '\n';
    std::string msg = std::string(buf);

    return msg;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <Server IP> <Server Port>\n";
        return 1;
    }

    // Extract server IP and port from command line arguments
    const char* serverIP = argv[1];
    short serverPort;
    std::sscanf(argv[2], "%hd", &serverPort);

    Socket clientSocket;
    struct sockaddr_in serverAddr;

    // Create a socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR_EXIT("socket");
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);

    // Convert server IP from string to binary
    if (inet_pton(AF_INET, serverIP, &serverAddr.sin_addr) <= 0) {
        ERR_EXIT("inet_pton");
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        ERR_EXIT("connect");
    }

    std::cout << "Connected to the server at " << serverIP << ":" << serverPort << "\n";

    // Interaction loop
    //char buffer[MAX_MSG_LEN] = {0};
    std::string message;
    /*
    message = readFromServer(clientSocket);
    
    std::cout << message;
    
    while (true) {
        std::string message;
        std::cin >> message;
        message.append("\r\n");
        writeToServer(clientSocket, message);

        char buf[MAX_MSG_LEN];
        scanf("%s", buf);
        strcat(buf, "\n");
        fprintf(stderr, "%s", buf);
        write(clientSocket, buf, strlen(buf));
        
        
        message = readFromServer(clientSocket);
        std::cout << message;
        
    }
    */
   
    while (true) {
        message = readFromServer(clientSocket);
        std::cout << message;
        if (message == "Please choose to Login, Logout or Register\n" || message == "Please enter Username: \n" || "Please enter Password: \n") {
            std::cin >> message;
            message.append("\r\n");
            writeToServer(clientSocket, message);
        } 
    }

    // Close the socket
    close(clientSocket);
    return 0;
}
