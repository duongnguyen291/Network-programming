# TEST 2.4 - QUICK START GUIDE

## 🚀 THỰC HIỆN NHANH

### 1. Compile
```cmd
cd week4\in-class\test4
gcc -o tcp_server_small tcp_server_small_buffer.c -lws2_32
gcc -o tcp_client_large tcp_client_large_message.c -lws2_32
```

### 2. Chạy Server (Terminal 1)
```cmd
tcp_server_small.exe
```

### 3. Chạy Client (Terminal 2)
```cmd
tcp_client_large.exe
```

## 📊 KẾT QUẢ MONG ĐỢI

### **Server Log:**
```
BUFF_SIZE = 7 bytes (small buffer for fragmentation test)
ret: 7 (recv #1, total: 7 bytes)
ret: 7 (recv #2, total: 14 bytes)
ret: 7 (recv #3, total: 21 bytes)
...
ret: 3 (recv #143, total: 1001 bytes)
Total fragments received: 143
```

### **Client Log:**
```
Sending large message (1000 characters + newline)...
Sent 1001 bytes to server
Received fragment #1: 7 bytes (total: 7/1001)
Received fragment #2: 7 bytes (total: 14/1001)
...
Total fragments received: 143
```

## 🎯 MỤC TIÊU
Chứng minh TCP message fragmentation - message lớn bị chia thành nhiều fragment nhỏ.

## 📝 GIẢI THÍCH
- **Small Buffer**: Server buffer 7 bytes
- **Large Message**: Client gửi 1000+ bytes
- **Fragmentation**: Message bị chia thành 143 fragment (7 bytes mỗi fragment)
- **Partial Reads**: Server nhận được nhiều lần, mỗi lần 7 bytes

## 🔍 KẾT LUẬN
- **TCP không đảm bảo message boundaries**
- **Large message có thể bị chia thành nhiều fragment**
- **Receiver phải xử lý partial reads**
- **Cần protocol layer để reassemble message**
