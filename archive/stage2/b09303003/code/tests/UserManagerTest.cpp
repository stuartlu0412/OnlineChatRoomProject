#include <gtest/gtest.h>
#include "UserManager.h"

// -----------------------------------------------------------------------------
// Test User Registration
// -----------------------------------------------------------------------------
TEST(UserManagerTest, RegisterUser) {
    UserManager userManager;

    // Register a new user
    EXPECT_TRUE(userManager.registerUser("alice", "password123"));
    // Attempt to register the same user again
    EXPECT_FALSE(userManager.registerUser("alice", "password123"));
    // Register another user
    EXPECT_TRUE(userManager.registerUser("bob", "securepass"));

    // Check the state of the UserManager
    User* alice = userManager.findUser("alice");
    ASSERT_NE(alice, nullptr);
    EXPECT_EQ(alice->username, "alice");
    EXPECT_FALSE(alice->isLoggedIn);

    User* bob = userManager.findUser("bob");
    ASSERT_NE(bob, nullptr);
    EXPECT_EQ(bob->username, "bob");
    EXPECT_FALSE(bob->isLoggedIn);
}

// -----------------------------------------------------------------------------
// Test User Login
// -----------------------------------------------------------------------------
TEST(UserManagerTest, LoginUser) {
    UserManager userManager;

    // Register users
    userManager.registerUser("alice", "password123");
    userManager.registerUser("bob", "securepass");

    // Login with valid credentials
    EXPECT_TRUE(userManager.loginUser("alice", "password123", "192.168.1.2", 5001));
    // Attempt login with invalid password
    EXPECT_FALSE(userManager.loginUser("alice", "wrongpassword", "192.168.1.2", 5001));
    // Attempt login for non-existent user
    EXPECT_FALSE(userManager.loginUser("charlie", "password123", "192.168.1.3", 5002));

    // Check the state of the UserManager
    User* alice = userManager.findUser("alice");
    ASSERT_NE(alice, nullptr);
    EXPECT_TRUE(alice->isLoggedIn);
    EXPECT_EQ(alice->ipAddress, "192.168.1.2");
    EXPECT_EQ(alice->port, 5001);
}

// -----------------------------------------------------------------------------
// Test User Logout
// -----------------------------------------------------------------------------
TEST(UserManagerTest, LogoutUser) {
    UserManager userManager;

    // Register and log in a user
    userManager.registerUser("alice", "password123");
    userManager.loginUser("alice", "password123", "192.168.1.2", 5001);

    // Logout the user
    EXPECT_TRUE(userManager.logoutUser("alice"));
    // Attempt logout for non-logged-in user
    EXPECT_FALSE(userManager.logoutUser("alice")); // Already logged out
    // Attempt logout for non-existent user
    EXPECT_FALSE(userManager.logoutUser("charlie"));

    // Check the state of the UserManager
    User* alice = userManager.findUser("alice");
    ASSERT_NE(alice, nullptr);
    EXPECT_FALSE(alice->isLoggedIn);
    EXPECT_EQ(alice->ipAddress, "");
    EXPECT_EQ(alice->port, 0);
}

// -----------------------------------------------------------------------------
// Test Retrieving Active Users
// -----------------------------------------------------------------------------
TEST(UserManagerTest, GetActiveUsers) {
    UserManager userManager;

    // Register users
    userManager.registerUser("alice", "password123");
    userManager.registerUser("bob", "securepass");

    // Log in some users
    userManager.loginUser("alice", "password123", "192.168.1.2", 5001);

    // Retrieve active users
    auto activeUsers = userManager.getActiveUsers();
    ASSERT_EQ(activeUsers.size(), 1);
    EXPECT_EQ(activeUsers[0].username, "alice");
    EXPECT_EQ(activeUsers[0].ipAddress, "192.168.1.2");
    EXPECT_EQ(activeUsers[0].port, 5001);

    // Log in another user
    userManager.loginUser("bob", "securepass", "192.168.1.3", 5002);
    activeUsers = userManager.getActiveUsers();
    ASSERT_EQ(activeUsers.size(), 2);

    // Ensure both users are listed
    EXPECT_TRUE(std::any_of(activeUsers.begin(), activeUsers.end(), [](const User& user) {
        return user.username == "alice" && user.ipAddress == "192.168.1.2" && user.port == 5001;
    }));
    EXPECT_TRUE(std::any_of(activeUsers.begin(), activeUsers.end(), [](const User& user) {
        return user.username == "bob" && user.ipAddress == "192.168.1.3" && user.port == 5002;
    }));
}

// -----------------------------------------------------------------------------
// Main Function for Google Test
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
