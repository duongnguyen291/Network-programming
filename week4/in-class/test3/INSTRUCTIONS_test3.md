# HƯỚNG DẪN THỰC HIỆN TEST 2.3: MULTIPLE MESSAGES COMBINED

## 🎯 MỤC TIÊU
Chứng minh rằng nhiều lần gọi `send()` nhỏ có thể được gộp thành một lần gọi `recv()` duy nhất.

## 📝 GIẢI THÍCH HIỆN TƯỢNG

### **TCP Buffering Behavior:**
- **TCP là stream protocol**: Không có ranh giới rõ ràng giữa các message
- **Nagle's Algorithm**: TCP có thể gộp nhiều segment nhỏ thành một segment lớn
- **OS Buffering**: Hệ điều hành có thể buffer dữ liệu trước khi gửi
- **Network Buffering**: Router/switch có thể gộp packet

### **Kết quả mong đợi:**
- Client gửi: `"bull"` + `"dog"` (2 lần send riêng biệt)
- Server nhận: `"bulldog"` (1 lần recv duy nhất)

## 🔧 CÁC BƯỚC THỰC HIỆN

### **Bước 1: Compile code**
```cmd
cd week4\in-class\test3
gcc -o tcp_server tcp_sever.c -lws2_32
gcc -o tcp_client_combined tcp_client_combined.c -lws2_32
```

### **Bước 2: Chạy Server**
Mở **Terminal 1**:
```cmd
cd week4\in-class\test3
tcp_server.exe
```
Kết quả mong đợi:
```
Server started. Waiting for connection on port 8080...
```

### **Bước 3: Chạy Client đặc biệt**
Mở **Terminal 2**:
```cmd
cd week4\in-class\test3
tcp_client_combined.exe
```

### **Bước 4: Quan sát kết quả**

## 📊 KẾT QUẢ MONG ĐỢI

### **Trường hợp 1: Messages được gộp (Phổ biến)**
```
=== TEST 2.3: Multiple Messages Combined ===
Sending 'bull'...
Sending 'dog'...
Waiting for server response...
Received from server: 'bulldog' (length: 7)
✓ RESULT: Messages were COMBINED into 'bulldog'
✓ This demonstrates TCP message merging behavior
```

**Server log:**
```
Server started. Waiting for connection on port 8080...
Client connected: 127.0.0.1:xxxxx
Received: bulldog
```

### **Trường hợp 2: Messages không được gộp (Ít gặp)**
```
=== TEST 2.3: Multiple Messages Combined ===
Sending 'bull'...
Sending 'dog'...
Waiting for server response...
Received from server: 'bull' (length: 4)
✓ RESULT: Only first message 'bull' received
✓ Waiting for second message...
Received second message: 'dog'
✓ RESULT: Messages were NOT combined
```

**Server log:**
```
Server started. Waiting for connection on port 8080...
Client connected: 127.0.0.1:xxxxx
Received: bull
Received: dog
```

## 🔍 GIẢI THÍCH KỸ THUẬT

### **Tại sao messages có thể được gộp?**

1. **TCP Stream Nature**: TCP không bảo toàn ranh giới message
2. **Nagle's Algorithm**: Gộp segment nhỏ để tối ưu bandwidth
3. **OS Socket Buffer**: Hệ điều hành buffer dữ liệu trước khi gửi
4. **Network Buffering**: Router có thể gộp packet
5. **Timing**: Nếu 2 send() gần nhau về thời gian

### **Các yếu tố ảnh hưởng:**
- **Kích thước message**: Message nhỏ dễ bị gộp
- **Thời gian giữa các send()**: Càng gần nhau càng dễ gộp
- **TCP_NODELAY**: Tắt Nagle's algorithm
- **Buffer size**: Kích thước buffer ảnh hưởng đến việc gộp

## 📸 EVIDENCE CẦN THU THẬP

1. **Client log**: Hiển thị messages gửi đi và nhận về
2. **Server log**: Hiển thị messages nhận được
3. **Timing**: Thời gian giữa các send() và recv()
4. **Buffer content**: Nội dung buffer sau mỗi recv()

## 🎯 KẾT LUẬN

Test này chứng minh rằng:
- **TCP không bảo toàn message boundaries**
- **Multiple send() có thể được gộp thành single recv()**
- **Ứng dụng cần tự quản lý message framing**
- **Cần protocol layer để phân tách message**

Đây là lý do tại sao các protocol như HTTP, FTP cần có delimiter hoặc length prefix để phân tách message.

## 🔧 TROUBLESHOOTING

### **Nếu không thấy message gộp:**
1. Thử chạy nhiều lần (timing có thể khác nhau)
2. Kiểm tra TCP_NODELAY setting
3. Thử với message lớn hơn
4. Kiểm tra network conditions

### **Nếu gặp lỗi compile:**
```cmd
gcc -o tcp_server tcp_sever.c -lws2_32
gcc -o tcp_client_combined tcp_client_combined.c -lws2_32
```

### **Nếu server không chạy:**
- Kiểm tra port 8080 có bị sử dụng không
- Đảm bảo firewall không block
- Thử restart terminal
