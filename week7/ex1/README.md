# Bài 1: Mã hóa/Giải mã File bằng Caesar Cipher với TCP và Select()

## Tổng quan
Đây là một ứng dụng mạng sử dụng socket TCP và hàm `select()` để xử lý nhiều clients cùng lúc, thực hiện mã hóa/giải mã file sử dụng thuật toán Caesar cipher.

## Đặc tả giao thức

### Định dạng thông điệp
```
[Opcode (1 byte)] [Length (2 bytes)] [Payload (biến đổi)]
```

### Giá trị Opcode
- **0**: Yêu cầu mã hóa
- **1**: Yêu cầu giải mã
- **2**: Truyền dữ liệu
- **3**: Thông báo lỗi

### Định dạng Payload
- **Opcode 0 hoặc 1**: Chứa giá trị khóa 4 byte (network byte order)
- **Opcode 2**: Chứa dữ liệu file (Length > 0) hoặc kết thúc truyền (Length = 0)
- **Opcode 3**: Không có payload

## Luồng giao thức

1. **Client** gửi yêu cầu mã hóa/giải mã với khóa (Opcode 0 hoặc 1)
2. **Client** gửi dữ liệu file theo từng khối (Opcode 2, Length > 0)
3. **Client** gửi tín hiệu kết thúc truyền (Opcode 2, Length = 0)
4. **Server** xử lý file và gửi dữ liệu đã mã hóa/giải mã (Opcode 2, Length > 0)
5. **Server** gửi tín hiệu kết thúc truyền (Opcode 2, Length = 0)
6. **Server** xóa file tạm và file đầu ra

## Biên dịch

### Biên dịch toàn bộ
```bash
make all
```

### Biên dịch riêng lẻ
```bash
make server
make client
```

### Dọn dẹp
```bash
make clean
```

## Hướng dẫn chạy

### Chạy Server
```bash
./server <port>
```

Ví dụ:
```bash
./server 5500
```

Server sẽ:
- Lắng nghe trên cổng được chỉ định
- Tạo thư mục `shared_files/` để lưu file đầu ra
- Tạo thư mục `temp_files/` để lưu file tạm
- Xử lý nhiều clients cùng lúc bằng `select()`

### Chạy Client
```bash
./client <IP_Server> <Port>
```

Ví dụ:
```bash
./client 127.0.0.1 5500
```

Client sẽ:
- Hiển thị menu lựa chọn mã hóa/giải mã
- Yêu cầu nhập khóa (0-25)
- Yêu cầu nhập tên file
- Gửi file tới server
- Nhận file đã xử lý lưu thành `received_file.txt`

## Tính năng chính

### Server
- **Multiplexing**: Sử dụng `select()` để xử lý nhiều clients cùng lúc
- **Quản lý file**: Tạo file tạm duy nhất cho mỗi yêu cầu
- **Xử lý lỗi**: Gửi thông báo lỗi (Opcode 3) khi có sự cố
- **Dọn dẹp**: Tự động xóa file tạm và file đầu ra sau truyền

### Client
- **Menu tương tác**: Giao diện thân thiện với người dùng
- **Kiểm tra file**: Xác minh file tồn tại trước khi gửi
- **Truyền an toàn**: Chia nhỏ file lớn (4093 byte/lần)
- **Nhận file**: Lưu file đã xử lý thành `received_file.txt`

## Thuật toán Caesar Cipher

- **Mã hóa**: Dịch chuyển ký tự chữ cái về phía trước theo giá trị khóa
- **Giải mã**: Dịch chuyển ký tự chữ cái về phía sau theo giá trị khóa
- **Ký tự khác**: Chữ số, dấu câu không được thay đổi
- **Bảo toàn ký tự**: Xử lý riêng cho chữ hoa và chữ thường

## Cấu trúc thư mục

```
week7/ex1/
├── server.c              # Mã nguồn server
├── client.c              # Mã nguồn client
├── Makefile              # Cấu hình biên dịch
├── README.md             # File này
├── shared_files/         # Thư mục chứa file đầu ra (tạo lúc chạy)
└── temp_files/           # Thư mục chứa file tạm (tạo lúc chạy)
```

## Ví dụ chạy ứng dụng

### Bước 1: Tạo file test (nếu chưa có)
```bash
echo "Hello World! This is a test message." > test_message.txt
```

Nội dung file:
```
Hello World! This is a test message.
```

### Bước 2: Chạy Server (Terminal 1)
```bash
$ ./server 5500
```

**Output:**
```
Server listening on port 5500
Client connected from 127.0.0.1:54321
Client 0: Request ENCODE with key 3
Client disconnected
```

### Bước 3: Chạy Client (Terminal 2)

#### TestCase 1: Mã hóa file (Encode)
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 1
Enter the key (0-25): 3
Enter the filename: test_message.txt
Sending ENCODE request with key 3 for file: test_message.txt
File sent. Waiting for server response...
File received successfully and saved as 'received_file.txt'

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

Kiểm tra file nhận được:
```bash
$ cat received_file.txt
```

**Output:**
```
Khoor Zruog! Wklv lv d whvw phvvdjh.
```

#### TestCase 2: Giải mã file (Decode)
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 2
Enter the key (0-25): 3
Enter the filename: received_file.txt
Sending DECODE request with key 3 for file: received_file.txt
File sent. Waiting for server response...
File received successfully and saved as 'received_file.txt'

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

Kiểm tra file:
```bash
$ cat received_file.txt
```

**Output:**
```
Hello World! This is a test message.
```

#### TestCase 3: Lỗi - file không tồn tại
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 1
Enter the key (0-25): 3
Enter the filename: nonexistent.txt
File not found: nonexistent.txt

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

#### TestCase 4: Lỗi - khóa không hợp lệ
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 1
Enter the key (0-25): 30
Invalid key. Please use a value between 0 and 25.

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

#### TestCase 5: Mã hóa với khóa khác nhau
Tạo file test khác:
```bash
echo "ABCXYZ abcxyz 123 !@#" > test2.txt
```

Chạy client với khóa 5:
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 1
Enter the key (0-25): 5
Enter the filename: test2.txt
Sending ENCODE request with key 5 for file: test2.txt
File sent. Waiting for server response...
File received successfully and saved as 'received_file.txt'

=== Caesar Cipher File Encoder/Decoder ===
1. Encode file
2. Decode file
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

Kiểm tra kết quả:
```bash
$ cat received_file.txt
```

**Output:**
```
FGHCDE fghcde 123 !@#
```

## Ghi chú quan trọng

- Ứng dụng xử lý một yêu cầu trên một client tại một thời điểm
- Giá trị khóa phải từ 0-25
- File lớn được truyền theo khối, mỗi khối 4093 byte
- Server sử dụng `select()` để xử lý nhiều clients hiệu quả
- Tất cả giá trị multi-byte được truyền theo network byte order

## Các giới hạn

- Chỉ xử lý một yêu cầu trên mỗi client (theo yêu cầu)
- Caesar cipher là thuật toán mã hóa đơn giản (không sử dụng trong thực tế)
- Không có xác thực hoặc mã hóa thực sự cho giao tiếp
