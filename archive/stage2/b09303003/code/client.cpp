#include "Client.h"
#include <iostream>
#include <string>
#include <sstream>


int main(int argc, char** argv) {
    try {
        // Initialize the client with a listening port, server IP, and server port
        Client client(atoi(argv[1]), "127.0.0.1", 8088);

        std::string command;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, command);

            if (command.starts_with("REGISTER")) {
                // Parse and handle REGISTER command
                std::istringstream iss(command);
                std::string cmd, username, password;
                iss >> cmd >> username >> password;

                if (username.empty() || password.empty()) {
                    std::cerr << "Usage: REGISTER <username> <password>" << std::endl;
                    continue;
                }

                if (client.registerWithServer(username, password)) {
                    std::cout << "Registration successful." << std::endl;
                } else {
                    std::cerr << "Registration failed." << std::endl;
                }
            } else if (command.starts_with("LOGIN")) {
                // Parse and handle LOGIN command
                std::istringstream iss(command);
                std::string cmd, username, password;
                iss >> cmd >> username >> password;

                if (username.empty() || password.empty()) {
                    std::cerr << "Usage: LOGIN <username> <password>" << std::endl;
                    continue;
                }

                if (client.loginToServer(username, password)) {
                    std::cout << "Login successful." << std::endl;
                } else {
                    std::cerr << "Login failed." << std::endl;
                }

            } else if (command.starts_with("LOGOUT")) {
                // Handle LOGOUT command
                if (client.logoutFromServer()) {
                    std::cout << "Logout successful." << std::endl;
                } else {
                    std::cerr << "Logout failed." << std::endl;
                }
            } else if (command.starts_with("CHAT")) {
                std::istringstream iss(command);
                std::string cmd, username;
                iss >> cmd >> username;

                if (username.empty()) {
                    std::cerr << "Usage: CHAT <username>" << std::endl;
                    continue;
                }

                if (!client.chatWithClient(username)) {
                    std::cerr << "Failed to chat with " << username << "." << std::endl;
                }
            } else if (command == "EXIT") {
                // Handle EXIT command
                std::cout << "Exiting..." << std::endl;
                break;
            } else {
                std::cerr << "Unknown command. Available commands: REGISTER, LOGIN, LOGOUT, EXIT." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

