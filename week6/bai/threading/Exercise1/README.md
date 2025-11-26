# Exercise 1 - TCP Echo Server with Threading

This application implements a TCP client-server system where the server converts messages to uppercase using **threading technique** (pthread).

## Requirements

- GCC compiler
- POSIX threads library (pthread)
- Linux/Unix environment

## Compilation

Use the provided Makefile to compile both server and client:

```bash
make
```

This will create two executables:
- `server` - The TCP server
- `client` - The TCP client

To clean compiled files:

```bash
make clean
```

## Running the Application

### Step 1: Start the Server

Open a terminal and run:

```bash
./server
```

**Expected Output:**
```
Server is listening on 127.0.0.1:5500
Using threading technique for handling multiple clients
```

The server will now wait for client connections on IP `127.0.0.1` and port `5500`.

### Step 2: Start the Client(s)

Open one or more additional terminals and run:

```bash
./client
```

**Expected Output:**
```
Da ket noi thanh cong den server 127.0.0.1:5500
Nhap tin nhan de gui. Nhap 'q' hoac 'Q' de thoat.
Nhap: 
```

### Step 3: Send Messages

Type any message and press Enter. The server will convert it to uppercase and send it back.

**Example Session:**

```
Nhap: hello world
Nhan tu server: HELLO WORLD
Nhap: Network Programming
Nhan tu server: NETWORK PROGRAMMING
Nhap: q

Tong so byte da gui: 33
```

## Input/Output Examples

### Example 1: Normal Messages

**Client Input:**
```
hello world
```

**Server Response:**
```
HELLO WORLD
```

### Example 2: Mixed Case

**Client Input:**
```
This is a TEST message
```

**Server Response:**
```
THIS IS A TEST MESSAGE
```

### Example 3: Quit Command

**Client Input:**
```
q
```

**Client Output:**
```
Tong so byte da gui: 45
```

**Server Output:**
```
Client 127.0.0.1:xxxxx disconnected
```

## Multiple Clients

The server uses **threading** to handle multiple clients simultaneously. You can run multiple client instances:

**Terminal 1 (Server):**
```
./server
Server is listening on 127.0.0.1:5500
Using threading technique for handling multiple clients
Client connected from 127.0.0.1:45678
Client connected from 127.0.0.1:45679
Client connected from 127.0.0.1:45680
```

**Terminal 2 (Client 1):**
```
./client
Da ket noi thanh cong den server 127.0.0.1:5500
Nhap: client one
Nhan tu server: CLIENT ONE
```

**Terminal 3 (Client 2):**
```
./client
Da ket noi thanh cong den server 127.0.0.1:5500
Nhap: client two
Nhan tu server: CLIENT TWO
```

Each client operates independently thanks to threading!

## Technical Details

### Server Implementation
- Uses `pthread_create()` to spawn a new thread for each client
- Threads are detached with `pthread_detach()` for automatic cleanup
- Each thread handles one client's connection independently
- IP address: `127.0.0.1` (localhost)
- Port: `5500`

### Client Implementation
- Connects to server at `127.0.0.1:5500`
- Tracks total bytes sent
- Handles connection failures with error messages
- Terminates on 'q' or 'Q' command

## Error Handling

### Connection Failed

If the server is not running:

```
connect: Connection refused
Khong the ket noi den server 127.0.0.1:5500
```

**Solution:** Start the server first.

### Port Already in Use

If port 5500 is already in use:

```
bind: Address already in use
```

**Solution:** Stop the other process using port 5500 or modify the `SERVER_PORT` in both files.

## File Structure

```
Exercise1/
├── server.c      # Server implementation with threading
├── client.c      # Client implementation
├── Makefile      # Build configuration
└── README.md     # This file
```

## Notes

- The server will continue running until manually stopped (Ctrl+C)
- Each client connection runs in a separate thread
- The server can handle multiple clients simultaneously
- Total bytes sent includes all characters including newlines
