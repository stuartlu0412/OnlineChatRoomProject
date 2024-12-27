#include "UserManager.h"
#include <iostream> // For debugging/logging
#include <stdexcept> // For exceptions


bool UserManager::registerUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(userMutex_);
    if (userDatabase_.find(username) != userDatabase_.end()) {
        return false; // User already exists
    }

    User newUser = {username, hashPassword(password), "", 0, false};
    userDatabase_[username] = newUser;
    return true;
}

bool UserManager::loginUser(const std::string& username, const std::string& password, const std::string& ipAddress, uint16_t port) {
    std::lock_guard<std::mutex> lock(userMutex_);
    auto it = userDatabase_.find(username);
    if (it == userDatabase_.end() || hashPassword(password) != it->second.passwordHash) {
        return false; // Invalid credentials
    }

    it->second.isLoggedIn = true;
    it->second.ipAddress = ipAddress;
    it->second.port = port;
    return true;
}

bool UserManager::logoutUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(userMutex_);
    auto it = userDatabase_.find(username);
    if (it == userDatabase_.end() || !it->second.isLoggedIn) {
        return false; // User not found or not logged in
    }

    it->second.isLoggedIn = false;
    it->second.ipAddress = "";
    it->second.port = 0;
    return true;
}

std::vector<User> UserManager::getActiveUsers() {
    std::lock_guard<std::mutex> lock(userMutex_);
    std::vector<User> activeUsers;

    for (const auto& [username, user] : userDatabase_) {
        if (user.isLoggedIn) {
            activeUsers.push_back(user);
        }
    }

    return activeUsers;
}

//bool UserManager::isLoggedIn(const std::string& username) {
 //   std::lock_guard<std::mutex> lock(userMutex_);
 //   auto it = userDatabase_.find(username);
 //   return it != userDatabase_.end() && it->second.isLoggedIn;
//}

User* UserManager::findUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(userMutex_);
    auto it = userDatabase_.find(username);
    return (it != userDatabase_.end() && it->second.isLoggedIn) ? &it->second : nullptr;
}

std::string UserManager::hashPassword(const std::string& password) {
    // Replace this with a real hashing function (e.g., bcrypt, Argon2, or SHA-256).
    return "hashed_" + password; // Placeholder for demonstration purposes
}
