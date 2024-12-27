#include "Server.h"
#include <iostream>

int main() {
    const int port = 8088;          // Port to listen on
    const size_t threadCount = 10;  // Number of threads in the ThreadPool

    try {
        Server server(port, threadCount);
        std::cout << "Server is running on port " << port << std::endl;
        server.start();  // Start the server and accept connections
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
