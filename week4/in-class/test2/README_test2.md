# TEST 2.2: MULTIPLE CONCURRENT CLIENTS

## MỤC TIÊU
Quan sát cách TCP server xử lý nhiều kết nối đồng thời từ nhiều client.

## GIẢI THÍCH HÀNH VI DỰ KIẾN
Server hiện tại chỉ xử lý được **1 client tại một thời điểm**. Khi có nhiều client kết nối:
- Client đầu tiên sẽ được xử lý bình thường
- Các client khác sẽ **PHẢI CHỜ** cho đến khi client đầu tiên kết thúc
- Đây là hành vi **tuần tự (sequential)**, không phải **đồng thời (concurrent)**

## CÁC BƯỚC THỰC HIỆN

### Bước 1: Compile code
```cmd
cd week4\in-class\test2
gcc -o tcp_server tcp_sever.c -lws2_32
gcc -o tcp_client tcp_client.c -lws2_32
```

### Bước 2: Chạy Server
Mở **Terminal 1**:
```cmd
cd week4\in-class\test2
tcp_server.exe
```
Kết quả mong đợi:
```
Server started. Waiting for connection on port 8080...
```

### Bước 3: Chạy Client 1
Mở **Terminal 2**:
```cmd
cd week4\in-class\test2
tcp_client.exe
```
Nhập tin nhắn: `Client 1 - Message 1`

### Bước 4: Chạy Client 2 (TRƯỚC KHI CLIENT 1 KẾT THÚC)
Mở **Terminal 3**:
```cmd
cd week4\in-class\test2
tcp_client.exe
```
**QUAN SÁT:** Client 2 sẽ kết nối được nhưng **KHÔNG THỂ GỬI TIN NHẮN** vì server đang bận xử lý Client 1.

### Bước 5: Chạy Client 3
Mở **Terminal 4**:
```cmd
cd week4\in-class\test2
tcp_client.exe
```
**QUAN SÁT:** Client 3 cũng sẽ kết nối được nhưng **PHẢI CHỜ**.

### Bước 6: Kết thúc Client 1
Trong Terminal 2, nhập đủ 3 tin nhắn để Client 1 kết thúc.

### Bước 7: Quan sát Client 2 và 3
Sau khi Client 1 kết thúc:
- Client 2 sẽ bắt đầu hoạt động
- Client 3 vẫn phải chờ Client 2 kết thúc

## KẾT QUẢ MONG ĐỢI

### Server Log:
```
Server started. Waiting for connection on port 8080...
Client connected: 127.0.0.1:xxxxx
Received: Client 1 - Message 1
Received: Client 1 - Message 2
Received: Client 1 - Message 3
Client connected: 127.0.0.1:yyyyy
Received: Client 2 - Message 1
Received: Client 2 - Message 2
Received: Client 2 - Message 3
Client connected: 127.0.0.1:zzzzz
Received: Client 3 - Message 1
Received: Client 3 - Message 2
Received: Client 3 - Message 3
```

### Giải thích hành vi:
1. **Server tuần tự:** Server chỉ xử lý 1 client tại một thời điểm
2. **Blocking accept:** Hàm `accept()` chỉ được gọi 1 lần, sau đó server vào vòng lặp xử lý client
3. **Không có threading:** Server không tạo thread riêng cho mỗi client
4. **FIFO (First In, First Out):** Client nào kết nối trước sẽ được xử lý trước

## EVIDENCE CẦN THU THẬP

1. **Screenshots:** Chụp ảnh màn hình cả 4 terminal
2. **Server log:** Ghi lại thứ tự nhận tin nhắn
3. **Client behavior:** Quan sát client nào hoạt động trước/sau
4. **Timing:** Ghi lại thời gian kết nối và gửi tin nhắn

## KẾT LUẬN
Test này chứng minh rằng server hiện tại **KHÔNG HỖ TRỢ** xử lý đồng thời nhiều client. Để hỗ trợ concurrent clients, cần:
- Sử dụng threading (pthread, Windows threads)
- Hoặc sử dụng non-blocking I/O
- Hoặc sử dụng select()/poll() để multiplexing
