# TEST 2.3: MULTIPLE MESSAGES COMBINED AT RECEIVER

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

### Bước 1: Tạo Client đặc biệt
Tạo file `tcp_client_combined.c` với logic:
```c
// Gửi 2 message liên tiếp
send(sockfd, "bull", 4, 0);
send(sockfd, "dog", 3, 0);

// Nhận 1 lần duy nhất
recv(sockfd, buffer, 1024, 0);
```

### Bước 2: Sử dụng Server từ test1
Server không cần thay đổi, vì nó đã có logic echo.

### Bước 3: Chạy test
1. Chạy server
2. Chạy client đặc biệt
3. Quan sát kết quả

## 📊 KẾT QUẢ MONG ĐỢI

### **Trường hợp 1: Messages được gộp**
```
Client sends: "bull" + "dog"
Server receives: "bulldog"
Client receives: "bulldog"
```

### **Trường hợp 2: Messages không được gộp**
```
Client sends: "bull" + "dog"  
Server receives: "bull"
Server receives: "dog"
Client receives: "bull"
Client receives: "dog"
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

1. **Client log**: Hiển thị messages gửi đi
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
