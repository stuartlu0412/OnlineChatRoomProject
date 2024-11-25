#define MAX_MSG_LEN 128
#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring> // For memset
#include <sys/select.h>

using Socket = int;
using IPString = std::string;
using Port = int;

class Client {
private:

    Port serverPort_;
    IPString serverIP_;
    Socket serverSocket_;
    void writeToServer();
    std::string readFromServer();
    
public:

    Client(Port serverPort, IPString serverIP)
        : serverPort_(serverPort)
        , serverIP_(serverIP)
    {}

    void run();

};

Client::Client(Port serverPort, IPString serverIP) {
    // initialized the client
}

Client::run() {
    // read server message until there is nothing to read
    // write to server
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <Server IP> <Server Port>\n";
        return 1;
    }

}
