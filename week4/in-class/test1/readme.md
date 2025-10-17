Tôi sẽ hướng dẫn bạn thực hiện bài test 2.1 (Basic echo) step by step. Trước tiên, hãy để tôi xem xét code hiện tại của bạn.


[2 tools called]

Tuyệt vời! Code của bạn đã sẵn sàng để thực hiện test 2.1. Đây là hướng dẫn step by step:

## **BƯỚC 1: Compile các file**

Mở terminal và compile cả 2 file:

```bash
# Compile server
gcc -o tcp_server tcp_sever.c

# Compile client  
gcc -o tcp_client tcp_client.c
```

## **BƯỚC 2: Chạy Server trước**

Mở terminal đầu tiên và chạy server:

```bash
./tcp_server
```

Bạn sẽ thấy output:
```
Server started. Waiting for connection on port 8080...
```

## **BƯỚC 3: Chạy Client**

Mở terminal thứ hai và chạy client:

```bash
./tcp_client
```

Bạn sẽ thấy:
```
Connected to server!
Enter message 1: 
```

## **BƯỚC 4: Thực hiện test với 3 chuỗi**

Theo yêu cầu, bạn cần nhập 3 chuỗi tùy ý, trong đó có 1 chuỗi chứa khoảng trắng. Ví dụ:

**Chuỗi 1:** `Hello`
**Chuỗi 2:** `This is a test message` (có khoảng trắng)
**Chuỗi 3:** `Goodbye`

## **BƯỚC 5: Quan sát kết quả**

**Ở terminal Client:**
```
Connected to server!
Enter message 1: Hello
Echo from server: Hello

Enter message 2: This is a test message
Echo from server: This is a test message

Enter message 3: Goodbye
Echo from server: Goodbye
```

**Ở terminal Server:**
```
Server started. Waiting for connection on port 8080...
Client connected: 127.0.0.1:xxxxx
Received: Hello

Received: This is a test message

Received: Goodbye
```

## **BƯỚC 6: Chụp ảnh màn hình**

Chụp ảnh màn hình cả 2 terminal để làm evidence cho báo cáo.

## **BƯỚC 7: Wireshark Capture (Tùy chọn)**

Nếu có Wireshark:
1. Mở Wireshark
2. Chọn interface loopback (lo0 hoặc lo)
3. Bắt đầu capture
4. Chạy lại test
5. Filter với: `tcp.port == 8080`
6. Dừng capture và lưu file

## **Lưu ý quan trọng:**

1. **Thứ tự chạy:** Luôn chạy server trước, sau đó mới chạy client
2. **Port:** Đảm bảo port 8080 không bị sử dụng bởi ứng dụng khác
3. **Kết thúc:** Sau khi test xong, client sẽ tự động đóng kết nối
4. **Server:** Server sẽ tiếp tục chạy cho đến khi bạn nhấn Ctrl+C

Bạn có muốn tôi hướng dẫn thêm về các test scenario khác (2.2, 2.3, 2.4) không?