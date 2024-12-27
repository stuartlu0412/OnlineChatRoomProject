import subprocess
import socket
import time

# Server connection details
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 8080  # Make sure this matches the port in main.cpp
SERVER_EXECUTABLE = "./main"  # Path to your server executable


def start_server():
    """
    Start the server using subprocess.
    """
    return subprocess.Popen([SERVER_EXECUTABLE], stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def client_task(client_id, message):
    """
    Simulate a client connecting to the server, sending a message,
    and verifying the "OK" response.
    """
    try:
        # Create a socket
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((SERVER_HOST, SERVER_PORT))
        print(f"[Client {client_id}] Connected to server.")

        # Send a message
        client_socket.sendall(message.encode('utf-8'))
        print(f"[Client {client_id}] Sent: {message}")

        # Receive a response
        response = client_socket.recv(1024).decode('utf-8')
        print(f"[Client {client_id}] Received: {response}")

        # Verify the response is "OK"
        assert response == "OK", f"[Client {client_id}] Expected 'OK', got '{response}'"

        # Close the connection
        client_socket.close()
        print(f"[Client {client_id}] Connection closed.")
    except Exception as e:
        print(f"[Client {client_id}] Error: {e}")


def main():
    # Start the server
    print("Starting server...")
    server_process = start_server()
    time.sleep(2)  # Give the server some time to start

    try:
        # Number of clients to simulate
        NUM_CLIENTS = 5
        MESSAGE = "Hello, Server!"

        # Simulate multiple clients
        for client_id in range(NUM_CLIENTS):
            client_task(client_id, MESSAGE)

    finally:
        # Stop the server
        print("Stopping server...")
        server_process.terminate()
        server_process.wait()
        print("Server stopped.")


if __name__ == "__main__":
    main()
