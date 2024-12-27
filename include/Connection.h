#ifndef CONNECTION_H
#define CONNECTION_H

#include <sys/socket.h> // for socket functions/types if needed
#include <netinet/in.h> // for sockaddr_in, etc.
#include <unistd.h>     // for close()
#include <string>

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#endif

using Socket = int;

class Connection
{
public:
    /**
     * @brief Constructs a Connection object with the given socket descriptor.
     * 
     * @param socketFd The socket file descriptor for this connection.
     * @param clientAddr (optional) The client’s address if you want to store it.
     */
    Connection(Socket socketFd, const sockaddr_in& clientAddr);

    /**
     * @brief Destroys the Connection object, closing socket if still open.
     */
    ~Connection();

    /**
     * @brief Handles interaction with this client—reading, writing,
     *        processing messages, etc.
     * 
     * You might call this from a worker thread in the ThreadPool.
     */
    void handleClient();

    /**
     * @brief Closes the connection if it is still open.
     */
    void closeConnection();

#ifdef USE_OPENSSL
    /**
     * @brief Set up SSL for this connection. Perform the SSL handshake, etc.
     *
     * @param sslContext The server’s SSL context.
     * @return true on successful SSL handshake, false on error.
     */
    bool setupSSL(SSL_CTX* sslContext);
#endif

private:
    /**
     * @brief The raw socket file descriptor for this client.
     */
    int socketFd_;

    /**
     * @brief The client’s address (useful if you need the IP/port).
     */
    sockaddr_in clientAddr_;

#ifdef USE_OPENSSL
    /**
     * @brief The SSL handle for this connection (nullptr if not using SSL).
     */
    SSL* sslHandle_;
#endif

    /**
     * @brief Helper function to read data from the socket (or SSL handle).
     *
     * @param buffer The buffer to store received data.
     * @param size   The maximum number of bytes to read.
     * @return Number of bytes read, or -1 on error.
     */
    ssize_t receiveData(char* buffer, size_t size);

    /**
     * @brief Helper function to write data to the socket (or SSL handle).
     *
     * @param data The data to send.
     * @param size The number of bytes to send.
     * @return Number of bytes written, or -1 on error.
     */
    ssize_t sendData(const char* data, size_t size);

    /**
     * @brief Process incoming data (e.g., parse commands, send responses).
     *        You might define a protocol for how clients communicate.
     *
     * @param data The incoming data buffer.
     * @param size The size of the data.
     */
    void processData(const char* data, size_t size);

    /**
     * @brief A flag or state variable you might track.
     *        For example, whether the client is still connected,
     *        or if the handshake is done, etc.
     */
    bool connected_;
};

#endif // CONNECTION_H
