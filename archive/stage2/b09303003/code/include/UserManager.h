#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <unordered_map>
#include <mutex>
#include <vector>
#include <string>

struct User {
    std::string username;       // Unique identifier for the user
    std::string passwordHash;   // Securely hashed password
    std::string ipAddress;      // Client's IP address
    uint16_t port;              // Client's port for P2P
    bool isLoggedIn = false;    // Indicates if the user is currently logged in
};

/**
 * @brief A class to manage users, including registration, login/logout, and active user tracking.
 */
class UserManager {
public:
    /**
     * @brief Registers a new user.
     * 
     * @param username The username to register.
     * @param password The plaintext password (hashed internally).
     * @return true if registration is successful, false if the username already exists.
     */
    bool registerUser(const std::string& username, const std::string& password);

    /**
     * @brief Logs a user in, storing their IP and port for P2P connections.
     * 
     * @param username The username to log in.
     * @param password The plaintext password to verify.
     * @param ipAddress The client's IP address.
     * @param port The client's port for P2P.
     * @return true if login is successful, false otherwise.
     */
    bool loginUser(const std::string& username, const std::string& password, const std::string& ipAddress, uint16_t port);

    /**
     * @brief Logs a user out.
     * 
     * @param username The username to log out.
     * @return true if the user was logged out successfully, false otherwise.
     */
    bool logoutUser(const std::string& username);

    /**
     * @brief Retrieves a list of active users and their P2P connection info.
     * 
     * @return A vector of active users with IP and port information.
     */
    std::vector<User> getActiveUsers();

    /**
     * @brief Finds a user by their username.
     * 
     * @param username The username to search for.
     * @return A pointer to the User object if found, or nullptr otherwise.
     */
    User* findUser(const std::string& username);

    //bool UserManager::isLoggedIn(const std::string& username);

private:
    /**
     * @brief Stores all registered users.
     */
    std::unordered_map<std::string, User> userDatabase_;

    /**
     * @brief Mutex to protect access to userDatabase_.
     */
    std::mutex userMutex_;

    /**
     * @brief A helper function to hash passwords.
     * 
     * @param password The plaintext password.
     * @return A hashed version of the password.
     */
    std::string hashPassword(const std::string& password);
};

#endif // USER_MANAGER_H
