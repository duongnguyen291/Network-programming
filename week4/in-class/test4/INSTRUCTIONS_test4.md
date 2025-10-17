# HƯỚNG DẪN THỰC HIỆN TEST 2.4: FRAGMENTATION WITH SMALL BUFFER

## 🎯 MỤC TIÊU
Chứng minh partial reads - một lần gửi lớn có thể bị chia thành nhiều lần nhận nhỏ.

## 📝 GIẢI THÍCH HIỆN TƯỢNG

### **TCP Fragmentation Behavior:**
- **Large Message**: Client gửi 1 message lớn (1000+ ký tự)
- **Small Buffer**: Server có buffer nhỏ (7 bytes)
- **Partial Reads**: Server nhận được nhiều lần, mỗi lần 7 bytes
- **Fragmentation**: Message lớn bị chia thành nhiều fragment nhỏ

### **Kết quả mong đợi:**
- Client gửi: 1 message lớn (1000 ký tự)
- Server nhận: Nhiều lần, mỗi lần 7 bytes
- Log: `ret: 7, ret: 7, ret: 7, ..., ret: <7` (lần cuối)

## 🔧 CÁC BƯỚC THỰC HIỆN

### **Bước 1: Compile code**
```cmd
cd week4\in-class\test4
gcc -o tcp_server_small tcp_server_small_buffer.c -lws2_32
gcc -o tcp_client_large tcp_client_large_message.c -lws2_32
```

### **Bước 2: Chạy Server**
Mở **Terminal 1**:
```cmd
cd week4\in-class\test4
tcp_server_small.exe
```
Kết quả mong đợi:
```
Server started. Waiting for connection on port 8080...
BUFF_SIZE = 7 bytes (small buffer for fragmentation test)
```

### **Bước 3: Chạy Client**
Mở **Terminal 2**:
```cmd
cd week4\in-class\test4
tcp_client_large.exe
```

### **Bước 4: Quan sát kết quả**

## 📊 KẾT QUẢ MONG ĐỢI

### **Server Log:**
```
Server started. Waiting for connection on port 8080...
BUFF_SIZE = 7 bytes (small buffer for fragmentation test)
Client connected: 127.0.0.1:xxxxx
=== TEST 2.4: Fragmentation with Small Buffer ===
Waiting for large message...
ret: 7 (recv #1, total: 7 bytes)
Received data: 'ABCDEFG'
ret: 7 (recv #2, total: 14 bytes)
Received data: 'HIJKLMN'
ret: 7 (recv #3, total: 21 bytes)
Received data: 'OPQRSTU'
...
ret: 3 (recv #143, total: 1001 bytes)
Received data: 'XYZ'
Complete message received!
=== TEST COMPLETED ===
Total fragments received: 143
Total bytes received: 1001
This demonstrates TCP message fragmentation with small buffer
```

### **Client Log:**
```
Connected to server!
=== TEST 2.4: Large Message Fragmentation ===
Sending large message (1000 characters + newline)...
Message preview: ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ...
Sent 1001 bytes to server
Waiting for server echo...
Received fragment #1: 7 bytes (total: 7/1001)
Fragment content: 'ABCDEFG'
Received fragment #2: 7 bytes (total: 14/1001)
Fragment content: 'HIJKLMN'
...
Complete message received!
=== TEST COMPLETED ===
Total fragments received: 143
Total bytes received: 1001
This demonstrates TCP message fragmentation behavior
```

## 🔍 GIẢI THÍCH KỸ THUẬT

### **Tại sao xảy ra fragmentation?**

1. **Buffer Size Limitation**: Server buffer (7 bytes) nhỏ hơn message size (1001 bytes)
2. **TCP Segment Size**: TCP có giới hạn segment size (MTU ~1500 bytes)
3. **OS Socket Buffer**: Hệ điều hành có giới hạn buffer
4. **Network MTU**: Router có giới hạn packet size
5. **Flow Control**: TCP flow control có thể chia nhỏ data

### **Các yếu tố ảnh hưởng:**
- **BUFF_SIZE**: Kích thước buffer của receiver
- **Message Size**: Kích thước message gửi đi
- **MTU**: Maximum Transmission Unit của network
- **TCP Window**: TCP window size
- **OS Limits**: Giới hạn của hệ điều hành

## 📸 EVIDENCE CẦN THU THẬP

1. **Server log**: Hiển thị số bytes nhận được mỗi lần (`ret: X`)
2. **Client log**: Hiển thị message gửi đi và nhận về
3. **Fragment count**: Tổng số fragment nhận được
4. **Total bytes**: Tổng số bytes nhận được
5. **Timing**: Thời gian giữa các recv()

## 🎯 KẾT LUẬN

Test này chứng minh rằng:
- **TCP không đảm bảo message boundaries**
- **Large message có thể bị chia thành nhiều fragment**
- **Receiver phải xử lý partial reads**
- **Ứng dụng cần implement message reassembly**

### **Tại sao cần protocol layer?**
Vì TCP không đảm bảo message boundaries, nên cần:
- **Length prefix**: Để biết message size
- **Delimiter**: Để biết message boundary
- **Framing**: Để reassemble message
- **Buffer management**: Để xử lý partial reads

## 🔧 TROUBLESHOOTING

### **Nếu không thấy fragmentation:**
1. Kiểm tra BUFF_SIZE có đủ nhỏ không (7 bytes)
2. Kiểm tra message size có đủ lớn không (1000+ bytes)
3. Thử tăng message size lên 5000+ bytes
4. Kiểm tra network conditions

### **Nếu gặp lỗi compile:**
```cmd
gcc -o tcp_server_small tcp_server_small_buffer.c -lws2_32
gcc -o tcp_client_large tcp_client_large_message.c -lws2_32
```

### **Nếu server không chạy:**
- Kiểm tra port 8080 có bị sử dụng không
- Đảm bảo firewall không block
- Thử restart terminal
