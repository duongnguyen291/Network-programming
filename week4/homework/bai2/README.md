# BÀI TẬP 2: FILE TRANSFER TCP SERVER/CLIENT

## 📋 MÔ TẢ BÀI TẬP

Xây dựng ứng dụng TCP Server/Client để truyền file:
- **Server**: Nhận file từ client và lưu vào thư mục uploads/
- **Client**: Gửi file lên server và hiển thị kết quả
- **Xử lý lỗi**: Thông báo lỗi cho các trường hợp khác nhau
- **Hỗ trợ file lớn**: Lên đến 100MB

## 🎯 YÊU CẦU CHỨC NĂNG

### **Server:**
- Khởi động với port từ command line: `./server <port_number>`
- Tạo thư mục `uploads/` để lưu file
- Nhận file từ client
- Kiểm tra file đã tồn tại (tránh ghi đè)
- Lưu file vào thư mục uploads/
- Thông báo kết quả về client

### **Client:**
- Khởi động với IP và port: `./client <server_ip> <port_number>`
- Nhập đường dẫn file từ bàn phím
- Kiểm tra file tồn tại
- Gửi file lên server
- Hiển thị kết quả truyền file
- Lặp lại cho đến khi nhập đường dẫn rỗng

## 🔧 CÁCH COMPILE VÀ CHẠY

### **1. Compile:**
```bash
# Compile tất cả
make all

# Hoặc compile riêng
make server
make client

# Tạo file test
make test-files
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

### **Test Case 1: File transfer thành công**
```
Client Input: test_files/test.txt
Server Output: Transfer complete
Result: File được lưu vào uploads/test.txt
```

### **Test Case 2: File không tồn tại**
```
Client Input: nonexistent.txt
Server Output: Error: File not found
Result: Transfer failed
```

### **Test Case 3: File đã tồn tại trên server**
```
Client Input: test_files/test.txt (lần 2)
Server Output: Error: File already exists on server
Result: Transfer failed
```

### **Test Case 4: File quá lớn**
```
Client Input: large_file.bin (>100MB)
Server Output: Error: Invalid file size
Result: Transfer failed
```

## 🏗️ KIẾN TRÚC ỨNG DỤNG

### **Server Architecture:**
```
1. Khởi tạo socket và bind port
2. Tạo thư mục uploads/
3. Listen for connections
4. Accept client connection
5. While (client connected):
   a. Receive filename from client
   b. Check if file exists on server
   c. Receive file size from client
   d. Receive file data and save to uploads/
   e. Send result to client
6. Close connection
```

### **Client Architecture:**
```
1. Connect to server
2. While (user input not empty):
   a. Get file path from user
   b. Check if file exists locally
   c. Send filename to server
   d. Send file size to server
   e. Send file data to server
   f. Receive result from server
   g. Display result
3. Close connection
```

## 🔍 XỬ LÝ FILE TRANSFER

### **Protocol Design:**
1. **Client → Server**: Filename
2. **Server → Client**: OK/Error (file exists check)
3. **Client → Server**: File size
4. **Client → Server**: File data (chunked)
5. **Server → Client**: Transfer complete/Error

### **Error Handling:**
- **File not found**: Client kiểm tra file tồn tại
- **File already exists**: Server kiểm tra file đã tồn tại
- **File too large**: Kiểm tra kích thước file (max 100MB)
- **Transfer interrupted**: Xử lý lỗi kết nối
- **Invalid file size**: Kiểm tra kích thước hợp lệ

## 📁 CẤU TRÚC FILE

```
bai2/
├── server.c              # Server source code
├── client.c              # Client source code
├── Makefile              # Build configuration
├── README.md             # This file
├── test_files/           # Test files directory
│   ├── test.txt         # Test file 1
│   ├── sample.txt       # Test file 2
│   └── data.txt         # Test file 3
└── uploads/             # Server upload directory (created automatically)
```

## 🛠️ MAKE COMMANDS

```bash
make all          # Build both server and client
make server       # Build server only
make client       # Build client only
make clean        # Remove build artifacts and uploads
make test-files   # Create test files
make setup        # Create upload directory
make run-server   # Show server usage
make run-client   # Show client usage
make test         # Show testing instructions
make help         # Show all available commands
```

## 🧪 TESTING

### **Manual Testing:**
1. Start server: `./server 5500`
2. Start client: `./client 127.0.0.1 5500`
3. Test with various files:
   - `test_files/test.txt` → Should transfer successfully
   - `test_files/sample.txt` → Should transfer successfully
   - `nonexistent.txt` → Should show "File not found"
   - `test_files/test.txt` (again) → Should show "File already exists"
   - Empty path → Should close connection

### **Expected Results:**
- Server should create uploads/ directory
- Files should be saved in uploads/ directory
- Error handling should work correctly
- Progress should be displayed for large files
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

3. **File not found:**
   ```
   Error: File not found
   Solution: Check file path and permissions
   ```

4. **File already exists:**
   ```
   Error: File already exists on server
   Solution: Use different filename or delete existing file
   ```

5. **File too large:**
   ```
   Error: Invalid file size
   Solution: Use file smaller than 100MB
   ```

## 📝 NOTES

- **File size limit**: Maximum 100MB per file
- **Upload directory**: Files are saved in `uploads/` directory
- **Progress display**: Shows progress for files larger than 1MB
- **Error handling**: Comprehensive error checking and reporting
- **Cross-platform**: Works on Windows and Linux
- **Memory efficient**: Uses chunked transfer for large files

## 🎯 LEARNING OBJECTIVES

After completing this exercise, you should understand:
- TCP socket programming for file transfer
- File I/O operations in C
- Error handling in network programming
- Protocol design for file transfer
- Large file handling techniques
- Client-server communication patterns
- Makefile usage for C projects
