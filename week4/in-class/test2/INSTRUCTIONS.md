# HƯỚNG DẪN THỰC HIỆN TEST 2.2: MULTIPLE CONCURRENT CLIENTS

## 🎯 MỤC TIÊU
Quan sát cách TCP server xử lý nhiều kết nối đồng thời và giải thích hành vi.

## 📋 CHUẨN BỊ

### Files trong folder test2:
- `tcp_sever.c` - Server code
- `tcp_client.c` - Client code gốc  
- `tcp_client_identifiable.c` - Client có ID để dễ nhận biết
- `run_test2.bat` - Script compile và hướng dẫn
- `open_terminals.bat` - Script mở nhiều terminal tự động

## 🚀 CÁCH 1: THỰC HIỆN THỦ CÔNG

### Bước 1: Compile code
```cmd
cd week4\in-class\test2
gcc -o tcp_server tcp_sever.c -lws2_32
gcc -o tcp_client tcp_client.c -lws2_32
gcc -o tcp_client_id tcp_client_identifiable.c -lws2_32
```

### Bước 2: Mở 4 terminal
- **Terminal 1:** Server
- **Terminal 2:** Client A  
- **Terminal 3:** Client B
- **Terminal 4:** Client C

### Bước 3: Chạy theo thứ tự
1. **Terminal 1:** `tcp_server.exe`
2. **Terminal 2:** `tcp_client_id.exe ClientA`
3. **Terminal 3:** `tcp_client_id.exe ClientB` (chạy ngay sau Terminal 2)
4. **Terminal 4:** `tcp_client_id.exe ClientC` (chạy ngay sau Terminal 3)

### Bước 4: Quan sát hành vi
- Client A sẽ hoạt động bình thường
- Client B và C sẽ kết nối được nhưng **PHẢI CHỜ**
- Sau khi Client A kết thúc, Client B mới hoạt động
- Sau khi Client B kết thúc, Client C mới hoạt động

## 🚀 CÁCH 2: SỬ DỤNG SCRIPT TỰ ĐỘNG

### Chạy script:
```cmd
cd week4\in-class\test2
open_terminals.bat
```

Script sẽ tự động:
- Compile code
- Mở 4 terminal
- Hướng dẫn từng bước

## 📊 KẾT QUẢ MONG ĐỢI

### Server Log:
```
Server started. Waiting for connection on port 8080...
Client connected: 127.0.0.1:xxxxx
Received: [ClientA] Message 1
Received: [ClientA] Message 2  
Received: [ClientA] Message 3
Client connected: 127.0.0.1:yyyyy
Received: [ClientB] Message 1
Received: [ClientB] Message 2
Received: [ClientB] Message 3
Client connected: 127.0.0.1:zzzzz
Received: [ClientC] Message 1
Received: [ClientC] Message 2
Received: [ClientC] Message 3
```

### Client Logs:
- **Client A:** Hoạt động ngay lập tức
- **Client B:** Hiển thị "Connected" nhưng không thể gửi tin nhắn
- **Client C:** Hiển thị "Connected" nhưng không thể gửi tin nhắn

## 🔍 GIẢI THÍCH HÀNH VI

### Tại sao server không xử lý đồng thời?

1. **Blocking accept():** Server chỉ gọi `accept()` 1 lần
2. **Sequential processing:** Sau khi accept, server vào vòng lặp xử lý 1 client
3. **No threading:** Không có thread riêng cho mỗi client
4. **FIFO queue:** Client nào kết nối trước được xử lý trước

### Để hỗ trợ concurrent clients cần:
- **Threading:** Tạo thread riêng cho mỗi client
- **Non-blocking I/O:** Sử dụng select()/poll()
- **Async I/O:** Sử dụng epoll (Linux) hoặc IOCP (Windows)

## 📸 EVIDENCE CẦN THU THẬP

1. **Screenshots:** Chụp ảnh 4 terminal
2. **Server log:** Copy log từ server terminal
3. **Client behavior:** Ghi lại thứ tự hoạt động
4. **Timing:** Thời gian kết nối và gửi tin nhắn

## 📝 BÁO CÁO

Trong báo cáo, cần giải thích:
- Hành vi quan sát được
- Tại sao server không xử lý đồng thời
- Cách cải thiện để hỗ trợ concurrent clients
- So sánh với server đa luồng (nếu có)
