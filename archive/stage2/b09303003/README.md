# C++ Chatroom Project

## Compile
使用 Make 來 Compile
```
make
```
## Usage

開啟 Server 的指令如下
```
./server <portnumber>
```

開啟 Client 的指令如下

```
./client <listen port>
```

The client will prompt for commands that you can enter interactively in the terminal.

---

## **Commands**

### **REGISTER**

Register a new user with the server.

**Usage:**

```bash
REGISTER <username> <password>
```

**Example:**

```bash
REGISTER alice mypassword
```

If successful, the server responds with:

```
OK REGISTERED
```

### **LOGIN**

Log in to the server as a registered user. The command automatically sends the client's IP address and listening port to the server for P2P connections.

**Usage:**

```bash
LOGIN <username> <password>
```

**Example:**

```bash
LOGIN alice mypassword
```

If successful, the server responds with:

```
OK LOGIN
```

### **LOGOUT**

Log out from the server.

**Usage:**

```bash
LOGOUT
```

**Example:**

```bash
LOGOUT
```

If successful, the server responds with:

```
OK LOGOUT
```

### **CHAT**

Start a chat with another user by their username. The command queries the server for the target user's IP address and port, establishes a connection, and starts a bi-directional chat session.

**Usage:**

```bash
CHAT <username>
```

**Example:**

```bash
CHAT bob
```

If successful, you can start typing messages. Type `quit` to end the chat.

### **EXIT**

Exit the application.

**Usage:**

```bash
EXIT
```

---

##

---
