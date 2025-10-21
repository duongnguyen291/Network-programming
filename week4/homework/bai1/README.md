# BÀI TẬP 1: STRING PROCESSING TCP SERVER/CLIENT

## 📋 MÔ TẢ BÀI TẬP

Xây dựng ứng dụng TCP Server/Client để xử lý chuỗi:
- **Server**: Nhận chuỗi từ client, tách thành 2 chuỗi (alphabet và digit)
- **Client**: Gửi chuỗi lên server và nhận kết quả
- **Xử lý lỗi**: Thông báo lỗi nếu chuỗi chứa ký tự không hợp lệ

## 🎯 YÊU CẦU CHỨC NĂNG

### **Server:**
- Khởi động với port từ command line: `./server <port_number>`
- Nhận chuỗi từ client
- Tách chuỗi thành:
  - Chuỗi chỉ chứa ký tự alphabet (a-z, A-Z)
  - Chuỗi chỉ chứa ký tự digit (0-9)
- Gửi kết quả về client
- Thông báo lỗi nếu có ký tự không hợp lệ

### **Client:**
- Khởi động với IP và port: `./client <server_ip> <port_number>`
- Nhập chuỗi từ bàn phím
- Gửi chuỗi lên server
- Nhận và hiển thị kết quả
- Lặp lại cho đến khi nhập chuỗi rỗng

## 🔧 CÁCH COMPILE VÀ CHẠY

### **1. Compile:**
```bash
# Compile tất cả
make all

# Hoặc compile riêng
make server
make client
```

### **2. Chạy Server:**
```bash
# Khởi động server trên port 5500
./server 5500

# Hoặc port khác
./server 8080
```

### **3. Chạy Client:**
```bash
# Kết nối đến server local
./client 127.0.0.1 5500

# Kết nối đến server khác
./client 192.168.1.100 5500
```

## 📊 VÍ DỤ SỬ DỤNG

### **Test Case 1: Chuỗi hợp lệ**
```
Client Input: 1ab23c
Server Output: 
- Digits: 123
- Alphabet: abc
```

### **Test Case 2: Chuỗi có ký tự không hợp lệ**
```
Client Input: 123abc#
Server Output: Error
```

### **Test Case 3: Chỉ có alphabet**
```
Client Input: hello
Server Output:
- Digits: (empty)
- Alphabet: hello
```

### **Test Case 4: Chỉ có digit**
```
Client Input: 12345
Server Output:
- Digits: 12345
- Alphabet: (empty)
```

## 🏗️ KIẾN TRÚC ỨNG DỤNG

### **Server Architecture:**
```
1. Khởi tạo socket và bind port
2. Listen for connections
3. Accept client connection
4. While (client connected):
   a. Receive string from client
   b. Process string (separate alphabet/digits)
   c. Send result back to client
5. Close connection
```

### **Client Architecture:**
```
1. Connect to server
2. While (user input not empty):
   a. Get string from user
   b. Send string to server
   c. Receive result from server
   d. Display result
3. Close connection
```

## 🔍 XỬ LÝ TCP STREAMING

### **Vấn đề TCP Streaming:**
- TCP là stream protocol, không có message boundaries
- Multiple send() có thể được gộp thành single recv()
- Single send() có thể bị chia thành multiple recv()

### **Giải pháp trong bài này:**
- Sử dụng newline (`\n`) làm delimiter
- Client gửi chuỗi kết thúc bằng newline
- Server nhận đến khi gặp newline
- Đơn giản nhưng hiệu quả cho text-based protocol

## 📁 CẤU TRÚC FILE

```
bai1/
├── server.c          # Server source code
├── client.c          # Client source code
├── Makefile          # Build configuration
└── README.md         # This file
```

## 🛠️ MAKE COMMANDS

```bash
make all          # Build both server and client
make server       # Build server only
make client       # Build client only
make clean        # Remove build artifacts
make run-server   # Show server usage
make run-client   # Show client usage
make test         # Show testing instructions
make help         # Show all available commands
```

## 🧪 TESTING

### **Manual Testing:**
1. Start server: `./server 5500`
2. Start client: `./client 127.0.0.1 5500`
3. Test with various strings:
   - `1ab23c` → Should return digits and alphabet
   - `123abc#` → Should return error
   - `hello` → Should return only alphabet
   - `12345` → Should return only digits
   - Empty string → Should close connection

### **Expected Results:**
- Server should handle multiple clients sequentially
- Client should display results correctly
- Error handling should work for invalid characters
- Connection should close gracefully

## 🐛 TROUBLESHOOTING

### **Common Issues:**

1. **Port already in use:**
   ```
   Error: bind() failed: Address already in use
   Solution: Use different port or kill existing process
   ```

2. **Connection refused:**
   ```
   Error: connect() failed: Connection refused
   Solution: Make sure server is running first
   ```

3. **Invalid port number:**
   ```
   Error: Invalid port number
   Solution: Use port between 1-65535
   ```

4. **Compilation errors:**
   ```
   Solution: Make sure you have gcc and make installed
   On Windows: Install MinGW or use WSL
   ```

## 📝 NOTES

- **Cross-platform**: Code works on Windows and Linux
- **Error handling**: Comprehensive error checking
- **Memory safety**: Proper string handling and null termination
- **TCP streaming**: Handles TCP message boundaries correctly
- **User-friendly**: Clear prompts and error messages

## 🎯 LEARNING OBJECTIVES

After completing this exercise, you should understand:
- TCP socket programming basics
- Client-server communication patterns
- String processing in C
- TCP streaming issues and solutions
- Error handling in network programming
- Makefile usage for C projects
