# TEST 2.3 - QUICK START GUIDE

## 🚀 THỰC HIỆN NHANH

### 1. Compile
```cmd
cd week4\in-class\test3
gcc -o tcp_server tcp_sever.c -lws2_32
gcc -o tcp_client_combined tcp_client_combined.c -lws2_32
```

### 2. Chạy Server (Terminal 1)
```cmd
tcp_server.exe
```

### 3. Chạy Client (Terminal 2)
```cmd
tcp_client_combined.exe
```

## 📊 KẾT QUẢ MONG ĐỢI

### **Trường hợp 1: Messages được gộp (Phổ biến)**
```
Sending 'bull'...
Sending 'dog'...
Received from server: 'bulldog' (length: 7)
✓ RESULT: Messages were COMBINED into 'bulldog'
```

### **Trường hợp 2: Messages không được gộp**
```
Sending 'bull'...
Sending 'dog'...
Received from server: 'bull' (length: 4)
Received second message: 'dog'
✓ RESULT: Messages were NOT combined
```

## 🎯 MỤC TIÊU
Chứng minh TCP có thể gộp nhiều `send()` thành một `recv()` - điều này chứng minh TCP không bảo toàn message boundaries.

## 📝 GIẢI THÍCH
- **TCP là stream protocol**: Không có ranh giới rõ ràng giữa messages
- **Nagle's Algorithm**: Gộp segment nhỏ để tối ưu bandwidth
- **OS Buffering**: Hệ điều hành có thể buffer dữ liệu
- **Kết quả**: `"bull"` + `"dog"` → `"bulldog"` (gộp thành 1 message)
