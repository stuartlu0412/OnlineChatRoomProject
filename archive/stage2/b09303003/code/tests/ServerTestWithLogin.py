import socket
import time
from threading import Thread

# Server connection details
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 8088  # Make sure this matches the server's port

def client_task(client_id, message, expected_response):
    """
    A client task to connect to the server, send a message, and validate the response.
    """
    try:
        # Connect to the server
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
            client_socket.connect((SERVER_HOST, SERVER_PORT))
            print(f"[Client {client_id}] Connected to server.")

            # Send a message
            client_socket.sendall(message.encode('utf-8'))
            print(f"[Client {client_id}] Sent: {message}")

            # Receive the response
            response = client_socket.recv(1024).decode('utf-8')
            print(f"[Client {client_id}] Received: {response}")

            # Validate the response
            assert response == expected_response, f"[Client {client_id}] Expected '{expected_response}', got '{response}'"

            print(f"[Client {client_id}] Test passed.")

    except Exception as e:
        print(f"[Client {client_id}] Error: {e}")


def main():
    # Define test cases
    test_cases = [
        (1, "REGISTER alice password123", "OK REGISTERED"),
        (2, "LOGIN alice password123", "OK LOGIN"),
        (3, "LOGOUT alice", "OK LOGOUT"),
    ]

    '''
    # Start clients for each test case
    threads = []
    for client_id, message, expected_response in test_cases:
        thread = Thread(target=client_task, args=(client_id, message, expected_response))
        threads.append(thread)
        thread.start()
        time.sleep(0.1)  # Small delay between client starts

    # Wait for all threads to complete
    for thread in threads:
        thread.join()
    '''

    # Simulate 50 clients sending "GET_USERS"
    NUM_CLIENTS = 5
    MESSAGE = "GET_USERS"

    threads = []
    for i in range(NUM_CLIENTS):
        thread = Thread(target=client_task, args=(i, MESSAGE, ""))  # Expecting an empty or valid user list
        threads.append(thread)
        thread.start()
        time.sleep(0.01)  # Smaller delay for more rapid connections

    for thread in threads:
        thread.join()

    print("All tests completed.")


if __name__ == "__main__":
    main()