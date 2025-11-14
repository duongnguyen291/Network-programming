# APPLICATION-LAYER PROTOCOL DESIGN
## TCP Chat Application with User Authentication and Logging

**Student:** [Họ tên sinh viên]  
**Student ID:** [MSSV]  
**Course:** IT4062E - Network Programming  
**Assignment:** HW41 - TCP Chat Application  

---

## 1. OVERVIEW

### 1.1 Application Description
- **Purpose**: Simple TCP chat application with user authentication and message logging
- **Architecture**: Client-Server model
- **Transport Protocol**: TCP (reliable, connection-oriented)
- **Features**: 
  - User login/authentication
  - Message exchange
  - Individual log files per user
  - Persistent message storage

### 1.2 Key Requirements
1. Client can send either login name or message
2. Server accepts connection and stores login name
3. Server saves messages to log file
4. Each client has separate log file named after login name

---

## 2. PROTOCOL DESIGN

### 2.1 Message Format
All messages follow a structured format with message type and payload:

```
MESSAGE_FORMAT:
+------------------+------------------+------------------+
| Message Type (1) | Payload Length   | Payload Data     |
| (1 byte)         | (4 bytes)        | (variable)       |
+------------------+------------------+------------------+
```

### 2.2 Message Types
| Type | Value | Description |
|------|-------|-------------|
| LOGIN | 0x01 | Client login request |
| MESSAGE | 0x02 | Text message |
| ACK | 0x03 | Acknowledgment |
| ERROR | 0x04 | Error response |
| LOGOUT | 0x05 | Client logout |

### 2.3 Protocol Flow

#### 2.3.1 Login Process
```
Client → Server: LOGIN + username
Server → Client: ACK (if successful) or ERROR (if failed)
```

#### 2.3.2 Message Exchange
```
Client → Server: MESSAGE + text_content
Server → Client: ACK (message logged successfully)
```

#### 2.3.3 Logout Process
```
Client → Server: LOGOUT
Server → Client: ACK (connection closed)
```

---

## 3. DETAILED MESSAGE SPECIFICATIONS

### 3.1 LOGIN Message (Type: 0x01)
```
+------+----------+------------------+
| 0x01 | Length   | Username         |
|      | (4 bytes)| (variable)       |
+------+----------+------------------+
```

**Example:**
- Username: "john_doe"
- Message: `01 00 00 00 08 6A 6F 68 6E 5F 64 6F 65`

### 3.2 MESSAGE Message (Type: 0x02)
```
+------+----------+------------------+
| 0x02 | Length   | Message Text     |
|      | (4 bytes)| (variable)       |
+------+----------+------------------+
```

**Example:**
- Message: "Hello, World!"
- Message: `02 00 00 00 0D 48 65 6C 6C 6F 2C 20 57 6F 72 6C 64 21`

### 3.3 ACK Message (Type: 0x03)
```
+------+----------+------------------+
| 0x03 | Length   | Status Message   |
|      | (4 bytes)| (variable)       |
+------+----------+------------------+
```

**Example:**
- Status: "Login successful"
- Message: `03 00 00 00 0F 4C 6F 67 69 6E 20 73 75 63 63 65 73 73 66 75 6C`

### 3.4 ERROR Message (Type: 0x04)
```
+------+----------+------------------+
| 0x04 | Length   | Error Message    |
|      | (4 bytes)| (variable)       |
+------+----------+------------------+
```

**Example:**
- Error: "Invalid username"
- Message: `04 00 00 00 0F 49 6E 76 61 6C 69 64 20 75 73 65 72 6E 61 6D 65`

### 3.5 LOGOUT Message (Type: 0x05)
```
+------+----------+
| 0x05 | 00 00 00 00 |
+------+----------+
```

---

## 4. SERVER BEHAVIOR

### 4.1 Connection Management
1. **Accept Connection**: Server accepts TCP connection from client
2. **Wait for LOGIN**: Server waits for client to send LOGIN message
3. **Validate Username**: Check username format and availability
4. **Create Log File**: Create log file named `{username}.log`
5. **Send ACK/ERROR**: Respond with appropriate status

### 4.2 Message Processing
1. **Receive MESSAGE**: Get message from client
2. **Log to File**: Append message to user's log file with timestamp
3. **Send ACK**: Confirm message received and logged
4. **Handle LOGOUT**: Close connection and log file

### 4.3 Log File Format
```
[YYYY-MM-DD HH:MM:SS] {username}: {message}
```

**Example:**
```
[2024-01-15 14:30:25] john_doe: Hello, everyone!
[2024-01-15 14:30:45] john_doe: How is everyone doing?
[2024-01-15 14:31:02] john_doe: Goodbye!
```

---

## 5. CLIENT BEHAVIOR

### 5.1 Connection Process
1. **Connect to Server**: Establish TCP connection
2. **Send LOGIN**: Send username to server
3. **Wait for ACK**: Confirm login successful
4. **Start Chat**: Begin sending messages

### 5.2 Message Sending
1. **Get User Input**: Read message from user
2. **Send MESSAGE**: Send message to server
3. **Wait for ACK**: Confirm message logged
4. **Continue**: Repeat until user quits

### 5.3 Disconnection
1. **Send LOGOUT**: Notify server of disconnection
2. **Wait for ACK**: Confirm logout
3. **Close Connection**: Terminate TCP connection

---

## 6. ERROR HANDLING

### 6.1 Server-Side Errors
- **Invalid Username**: Username contains invalid characters
- **Username Too Long**: Username exceeds maximum length
- **File Creation Error**: Cannot create log file
- **Connection Lost**: Client disconnected unexpectedly

### 6.2 Client-Side Errors
- **Connection Failed**: Cannot connect to server
- **Login Failed**: Server rejected login
- **Message Send Failed**: Cannot send message to server
- **Server Disconnected**: Server closed connection

### 6.3 Error Response Format
```
ERROR_MESSAGE = "Error: " + error_description
```

**Examples:**
- `"Error: Invalid username format"`
- `"Error: Username too long"`
- `"Error: Cannot create log file"`
- `"Error: Connection lost"`

---

## 7. PROTOCOL IMPLEMENTATION NOTES

### 7.1 Data Types
- **Message Type**: 1 byte (unsigned char)
- **Payload Length**: 4 bytes (unsigned int, network byte order)
- **Payload Data**: Variable length (string)

### 7.2 Network Byte Order
- All multi-byte integers use network byte order (big-endian)
- Use `htonl()` and `ntohl()` for conversion

### 7.3 String Handling
- All strings are null-terminated
- UTF-8 encoding for international characters
- Maximum username length: 50 characters
- Maximum message length: 1000 characters

### 7.4 Timeout Handling
- **Connection Timeout**: 30 seconds
- **Message Timeout**: 10 seconds
- **Login Timeout**: 15 seconds

---

## 8. SECURITY CONSIDERATIONS

### 8.1 Input Validation
- Validate username format (alphanumeric + underscore only)
- Check message length limits
- Sanitize input to prevent injection attacks

### 8.2 File System Security
- Restrict log file access to server process only
- Validate file paths to prevent directory traversal
- Limit log file size to prevent disk space exhaustion

### 8.3 Network Security
- Implement connection rate limiting
- Add authentication mechanism (future enhancement)
- Use encrypted connections (future enhancement)

---

## 9. TESTING SCENARIOS

### 9.1 Normal Operation
1. Client connects and logs in
2. Client sends multiple messages
3. Client logs out
4. Verify log file creation and content

### 9.2 Error Cases
1. Invalid username format
2. Username too long
3. Message too long
4. Server disconnection
5. File system errors

### 9.3 Edge Cases
1. Empty username
2. Empty message
3. Special characters in username
4. Very long messages
5. Multiple clients with same username

---

## 10. FUTURE ENHANCEMENTS

### 10.1 Additional Features
- User authentication with password
- Message encryption
- File transfer capability
- Group chat functionality
- Message history retrieval

### 10.2 Performance Improvements
- Connection pooling
- Asynchronous message processing
- Database storage instead of file system
- Load balancing for multiple servers

---

## 11. CONCLUSION

This protocol design provides a solid foundation for implementing a TCP chat application with user authentication and message logging. The protocol is:

- **Simple**: Easy to implement and understand
- **Extensible**: Can be enhanced with additional features
- **Robust**: Includes comprehensive error handling
- **Efficient**: Minimal overhead with structured message format

The design ensures reliable message delivery, proper user identification, and persistent message storage while maintaining simplicity and ease of implementation.

---

**Document Version:** 1.0  
**Last Updated:** [Current Date]  
**Author:** [Student Name]  
**Student ID:** [Student ID]
