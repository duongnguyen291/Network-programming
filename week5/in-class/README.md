# TCP Chat Application Implementation
## HW41 - IT4062E Network Programming

**Implementation of the designed application-layer protocol for TCP chat application with user authentication and message logging.**

---

## 📋 OVERVIEW

This implementation follows the protocol design specified in `HW41_Protocol_Design.md` and provides:

- **User Authentication**: Login with username validation
- **Message Exchange**: Send and receive text messages
- **Message Logging**: Individual log files per user
- **Error Handling**: Comprehensive error management
- **Protocol Compliance**: Full implementation of designed protocol

---

## 🏗️ ARCHITECTURE

### Server Components
- **Connection Management**: Accept multiple client connections
- **Session Handling**: Manage individual client sessions
- **Message Processing**: Parse and process protocol messages
- **Log Management**: Create and maintain user log files
- **Error Handling**: Validate input and handle errors

### Client Components
- **Connection**: Connect to server
- **Authentication**: Login with username
- **Message Interface**: Interactive menu for user actions
- **Protocol Communication**: Send/receive protocol messages
- **Session Management**: Handle login/logout states

---

## 🔧 COMPILATION AND USAGE

### Build Application
```bash
# Compile both server and client
make all

# Or compile individually
make server
make client
```

### Run Server
```bash
# Start server on port 5500
./server 5500

# Or different port
./server 8080
```

### Run Client
```bash
# Connect to local server
./client 127.0.0.1 5500

# Connect to remote server
./client 192.168.1.100 5500
```

---

## 📊 PROTOCOL IMPLEMENTATION

### Message Format
```
+------------------+------------------+------------------+
| Message Type (1) | Payload Length   | Payload Data     |
| (1 byte)         | (4 bytes)        | (variable)       |
+------------------+------------------+------------------+
```

### Message Types
| Type | Value | Description | Implementation |
|------|-------|-------------|----------------|
| LOGIN | 0x01 | Client login request | ✅ Implemented |
| MESSAGE | 0x02 | Text message | ✅ Implemented |
| ACK | 0x03 | Acknowledgment | ✅ Implemented |
| ERROR | 0x04 | Error response | ✅ Implemented |
| LOGOUT | 0x05 | Client logout | ✅ Implemented |

### Protocol Flow
1. **Login Process**: Client → LOGIN → Server → ACK/ERROR
2. **Message Exchange**: Client → MESSAGE → Server → ACK
3. **Logout Process**: Client → LOGOUT → Server → ACK

---

## 🎯 FEATURES IMPLEMENTED

### ✅ Core Features
- **User Authentication**: Username validation and session management
- **Message Logging**: Individual log files with timestamps
- **Protocol Compliance**: Full implementation of designed protocol
- **Error Handling**: Comprehensive error management
- **Session Management**: Proper login/logout handling

### ✅ Technical Features
- **Network Byte Order**: Proper handling of multi-byte integers
- **Memory Management**: Proper allocation and deallocation
- **Input Validation**: Username and message validation
- **File System**: Log directory creation and management
- **Cross-platform**: Windows and Linux support

### ✅ Security Features
- **Input Validation**: Username format validation (alphanumeric + underscore)
- **Length Limits**: Maximum username (50) and message (1000) lengths
- **File Security**: Safe log file creation and management
- **Error Sanitization**: Proper error message handling

---

## 📁 FILE STRUCTURE

```
week5/in-class/
├── server.c              # Server implementation
├── client.c              # Client implementation
├── Makefile              # Build configuration
├── README.md             # This file
├── HW41_Protocol_Design.md # Protocol design document
└── logs/                 # Log directory (created by server)
    ├── user1.log         # User log files
    ├── user2.log
    └── ...
```

---

## 🧪 TESTING

### Manual Testing
1. **Start Server**: `./server 5500`
2. **Start Client**: `./client 127.0.0.1 5500`
3. **Test Sequence**:
   - Login with username
   - Send multiple messages
   - Logout
   - Check log files

### Test Cases
| Test Case | Expected Result |
|-----------|----------------|
| Valid login | Login successful |
| Invalid username | Error: Invalid username format |
| Send message | Message logged successfully |
| Logout | Logout successful |
| Multiple clients | Each has separate log file |

---

## 📝 LOG FILE FORMAT

### File Naming
- Format: `{username}.log`
- Location: `logs/` directory
- Example: `logs/john_doe.log`

### Log Entry Format
```
[YYYY-MM-DD HH:MM:SS] {username}: {message}
```

### Example Log Content
```
[2024-01-15 14:30:25] john_doe: Hello, everyone!
[2024-01-15 14:30:45] john_doe: How is everyone doing?
[2024-01-15 14:31:02] john_doe: Goodbye!
```

---

## 🔍 ERROR HANDLING

### Server Errors
- **Invalid Username**: Username format validation
- **Username Too Long**: Length limit enforcement
- **File Creation Error**: Log file creation failure
- **Connection Lost**: Client disconnection handling

### Client Errors
- **Connection Failed**: Server connection failure
- **Login Rejected**: Server authentication failure
- **Message Send Failed**: Network communication failure
- **Server Disconnected**: Server connection loss

### Error Response Format
```
ERROR_MESSAGE = "Error: " + error_description
```

---

## 🛠️ MAKE COMMANDS

```bash
make all          # Build both server and client
make server       # Build server only
make client       # Build client only
make clean        # Remove build artifacts and logs
make setup        # Create log directory
make run-server   # Show server usage
make run-client   # Show client usage
make test         # Show testing instructions
make help         # Show all available commands
```

---

## 📊 PERFORMANCE CONSIDERATIONS

### Memory Management
- Proper allocation and deallocation of message structures
- Efficient string handling and validation
- Minimal memory footprint per client session

### Network Efficiency
- Binary protocol format for minimal overhead
- Network byte order for cross-platform compatibility
- Efficient message parsing and construction

### File System
- Append-only log files for performance
- Automatic log directory creation
- Safe file handling with error checking

---

## 🔒 SECURITY IMPLEMENTATION

### Input Validation
- Username format validation (alphanumeric + underscore only)
- Message length limits (max 1000 characters)
- Username length limits (max 50 characters)
- Input sanitization to prevent injection attacks

### File System Security
- Restricted log file access to server process only
- Safe file path construction to prevent directory traversal
- Proper file permissions and error handling

### Network Security
- Connection validation and error handling
- Message format validation
- Proper session management

---

## 🎯 PROTOCOL COMPLIANCE

### ✅ Implemented Features
- **Message Format**: Binary protocol with type, length, and payload
- **Message Types**: All 5 message types implemented
- **Protocol Flow**: Complete login, message, logout flow
- **Error Handling**: Comprehensive error management
- **Log Format**: Timestamped log entries per user
- **Session Management**: Proper login/logout handling

### ✅ Validation
- **Username Format**: Alphanumeric + underscore only
- **Length Limits**: Enforced maximum lengths
- **Message Validation**: Proper message type handling
- **Error Responses**: Standardized error messages

---

## 🚀 USAGE EXAMPLES

### Server Usage
```bash
# Start server on port 5500
./server 5500

# Output:
# TCP Chat Server started on port 5500
# Log directory: logs/
# Waiting for connections...
```

### Client Usage
```bash
# Connect to server
./client 127.0.0.1 5500

# Menu options:
# 1. Login
# 2. Send Message
# 3. Logout
# 4. Exit
```

### Example Session
```
1. Client connects to server
2. Client logs in with username "john_doe"
3. Client sends message "Hello, World!"
4. Client sends message "How is everyone?"
5. Client logs out
6. Server creates logs/john_doe.log with messages
```

---

## 📈 FUTURE ENHANCEMENTS

### Potential Improvements
- **Database Storage**: Replace file system with database
- **Encryption**: Add message encryption
- **Authentication**: Password-based authentication
- **Group Chat**: Multi-user chat rooms
- **File Transfer**: File sharing capability
- **Message History**: Retrieve message history

### Performance Optimizations
- **Connection Pooling**: Efficient connection management
- **Asynchronous Processing**: Non-blocking I/O
- **Load Balancing**: Multiple server instances
- **Caching**: Message caching for performance

---

## 🎉 CONCLUSION

This implementation successfully realizes the designed application-layer protocol for a TCP chat application. The implementation provides:

- **Complete Protocol Compliance**: Full implementation of designed protocol
- **Robust Error Handling**: Comprehensive error management
- **Security Features**: Input validation and file system security
- **Cross-platform Support**: Windows and Linux compatibility
- **Easy Testing**: Simple build and test procedures

The application demonstrates proper network programming practices, protocol design implementation, and software engineering principles.

---

**Implementation Version:** 1.0  
**Protocol Version:** 1.0  
**Last Updated:** [Current Date]
