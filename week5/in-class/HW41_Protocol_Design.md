# HW41 - TCP Chat Application Protocol Design

**Student:** [Họ tên sinh viên]  
**Student ID:** [MSSV]  
**Course:** IT4062E - Network Programming  

---

## 1. APPLICATION OVERVIEW

### Purpose
Simple TCP chat application with user authentication and individual message logging.

### Key Features
- User login with username
- Message exchange between client and server
- Individual log files per user
- Persistent message storage

---

## 2. PROTOCOL SPECIFICATION

### 2.1 Message Format
```
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

---

## 3. PROTOCOL FLOW

### 3.1 Login Process
```
1. Client connects to server
2. Client → Server: LOGIN + username
3. Server → Client: ACK (success) or ERROR (failure)
4. Server creates log file: {username}.log
```

### 3.2 Message Exchange
```
1. Client → Server: MESSAGE + text_content
2. Server → Client: ACK (message logged)
3. Server appends to log file with timestamp
```

### 3.3 Logout Process
```
1. Client → Server: LOGOUT
2. Server → Client: ACK
3. Server closes connection and log file
```

---

## 4. MESSAGE EXAMPLES

### LOGIN Message
```
Type: 0x01
Length: 8
Payload: "john_doe"
```

### MESSAGE Message
```
Type: 0x02
Length: 13
Payload: "Hello, World!"
```

### ACK Message
```
Type: 0x03
Length: 15
Payload: "Login successful"
```

### ERROR Message
```
Type: 0x04
Length: 15
Payload: "Invalid username"
```

---

## 5. LOG FILE FORMAT

### File Naming
- Format: `{username}.log`
- Example: `john_doe.log`

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

## 6. ERROR HANDLING

### Server Errors
- Invalid username format
- Username too long (>50 chars)
- File creation error
- Connection lost

### Client Errors
- Connection failed
- Login rejected
- Message send failed
- Server disconnected

### Error Response Format
```
ERROR_MESSAGE = "Error: " + error_description
```

---

## 7. IMPLEMENTATION REQUIREMENTS

### Data Types
- Message Type: 1 byte (unsigned char)
- Payload Length: 4 bytes (unsigned int, network byte order)
- Payload Data: Variable length string

### Constraints
- Maximum username length: 50 characters
- Maximum message length: 1000 characters
- Username format: alphanumeric + underscore only
- All strings null-terminated

### Network Considerations
- Use network byte order for multi-byte integers
- Implement connection timeouts
- Handle partial message transmission
- Validate all input data

---

## 8. TESTING SCENARIOS

### Normal Operation
1. Client connects and logs in
2. Client sends multiple messages
3. Client logs out
4. Verify log file creation and content

### Error Cases
1. Invalid username format
2. Username too long
3. Message too long
4. Server disconnection
5. File system errors

---

## 9. SECURITY CONSIDERATIONS

### Input Validation
- Validate username format (alphanumeric + underscore only)
- Check message length limits
- Sanitize input to prevent injection attacks

### File System Security
- Restrict log file access to server process only
- Validate file paths to prevent directory traversal
- Limit log file size to prevent disk space exhaustion

---

## 10. CONCLUSION

This protocol design provides a simple yet robust foundation for implementing a TCP chat application with user authentication and message logging. The protocol ensures reliable message delivery, proper user identification, and persistent message storage while maintaining simplicity and ease of implementation.

**Key Benefits:**
- Simple message format
- Clear protocol flow
- Comprehensive error handling
- Extensible design
- Easy to implement and test

---

**Document Version:** 1.0  
**Last Updated:** [Current Date]
