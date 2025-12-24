# Bài 2: Hệ thống Đăng nhập/Đăng xuất với TCP và Poll()

## Tổng quan
Ứng dụng mạng sử dụng socket TCP và hàm `poll()` để xử lý nhiều clients, cho phép người dùng đăng nhập và đăng xuất. Hệ thống quản lý tài khoản, theo dõi số lần đăng nhập sai và tự động khóa tài khoản sau 3 lần thất bại.

## Yêu cầu

### Chức năng chính
- ✅ Mỗi client terminal chỉ có thể đăng nhập một tài khoản duy nhất
- ✅ Mỗi tài khoản có thể đăng nhập từ nhiều terminal khác nhau
- ✅ Tài khoản bị khóa sau 3 lần đăng nhập sai
- ✅ Thông tin tài khoản được lưu trong file `account.txt`

### Định dạng file account.txt
```
UserID Password Status
```
- **UserID**: Tên đăng nhập
- **Password**: Mật khẩu
- **Status**: 
  - `0` = Tài khoản bị khóa
  - `1` = Tài khoản hoạt động

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
./server <Port>
```

Ví dụ:
```bash
./server 5500
```

Server sẽ:
- Đọc file `account.txt` để load danh sách tài khoản
- Lắng nghe trên cổng được chỉ định
- Sử dụng `poll()` để xử lý nhiều clients
- Theo dõi số lần đăng nhập sai của mỗi tài khoản
- Tự động khóa tài khoản sau 3 lần sai

### Chạy Client
```bash
./client <IP_Address> <Port>
```

Ví dụ:
```bash
./client 127.0.0.1 5500
```

Client sẽ:
- Kết nối tới server
- Hiển thị menu đăng nhập/đăng xuất
- Gửi yêu cầu đến server
- Nhận và hiển thị kết quả

## Giao thức

### Các lệnh từ Client
1. **LOGIN username password**
   - Yêu cầu đăng nhập với username và password
   
2. **LOGOUT**
   - Yêu cầu đăng xuất

### Phản hồi từ Server
- **OK: [message]** - Thành công
- **ERROR: [message]** - Lỗi

### Các trường hợp lỗi
- Tài khoản không tồn tại
- Sai mật khẩu
- Tài khoản bị khóa
- Client đã đăng nhập
- Client chưa đăng nhập (khi logout)

## Ví dụ chạy ứng dụng

### Bước 1: Khởi động Server (Terminal 1)
```bash
$ ./server 5500
```

**Output:**
```
Loaded 7 accounts
Server listening on port 5500
New client connected from 127.0.0.1:54321 (slot 0)
```

### Bước 2: Kết nối Client (Terminal 2)

#### TestCase 1: Đăng nhập thành công
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500
Welcome! Please login.

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: admin
Enter password: admin123

Server response: OK: Login successful. Welcome admin!

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 2

Server response: OK: Logout successful. Goodbye admin!

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

**Server Log:**
```
Client 0: LOGIN admin admin123
User admin logged in from client 0
Client 0: LOGOUT
User admin logged out from client 0
Client 0 (admin) disconnected
```

#### TestCase 2: Sai mật khẩu
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500
Welcome! Please login.

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: user1
Enter password: wrongpass

Server response: ERROR: Wrong password. 2 attempts remaining

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: user1
Enter password: wrongpass2

Server response: ERROR: Wrong password. 1 attempts remaining

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: user1
Enter password: wrongpass3

Server response: ERROR: Wrong password. Account is now locked

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: user1
Enter password: pass123

Server response: ERROR: Account is locked

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

**Server Log:**
```
Client 0: LOGIN user1 wrongpass
Client 0: LOGIN user1 wrongpass2
Client 0: LOGIN user1 wrongpass3
Client 0: LOGIN user1 pass123
Client 0 disconnected
```

#### TestCase 3: Tài khoản không tồn tại
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500
Welcome! Please login.

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: nonexistent
Enter password: password

Server response: ERROR: Account does not exist

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

#### TestCase 4: Đăng nhập từ nhiều terminal cùng lúc

**Terminal 2:**
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500
Welcome! Please login.

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: alice
Enter password: alice123

Server response: OK: Login successful. Welcome alice!
```

**Terminal 3 (đồng thời):**
```bash
$ ./client 127.0.0.1 5500
```

**Output:**
```
Connected to server at 127.0.0.1:5500
Welcome! Please login.

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: alice
Enter password: alice123

Server response: OK: Login successful. Welcome alice!
```

**Kết quả:** Cả 2 terminal đều đăng nhập thành công với cùng tài khoản `alice`.

**Server Log:**
```
New client connected from 127.0.0.1:54321 (slot 0)
New client connected from 127.0.0.1:54322 (slot 1)
Client 0: LOGIN alice alice123
User alice logged in from client 0
Client 1: LOGIN alice alice123
User alice logged in from client 1
```

#### TestCase 5: Một client không thể đăng nhập nhiều tài khoản

**Output:**
```
Connected to server at 127.0.0.1:5500
Welcome! Please login.

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: admin
Enter password: admin123

Server response: OK: Login successful. Welcome admin!

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: user1
Enter password: pass123

Server response: ERROR: Already logged in as admin

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 2

Server response: OK: Logout successful. Goodbye admin!

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: user1
Enter password: pass123

Server response: OK: Login successful. Welcome user1!

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

#### TestCase 6: Logout khi chưa đăng nhập

**Output:**
```
Connected to server at 127.0.0.1:5500
Welcome! Please login.

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 2

Server response: ERROR: Not logged in

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

#### TestCase 7: Tài khoản bị khóa sẵn

**Output:**
```
Connected to server at 127.0.0.1:5500
Welcome! Please login.

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 1
Enter username: locked_user
Enter password: password

Server response: ERROR: Account is locked

=== User Authentication System ===
1. Login
2. Logout
3. Exit
Select an option (1-3): 3
Exiting...
Connection closed.
```

## Cấu trúc thư mục

```
week7/ex2/
├── server.c          # Mã nguồn server với poll()
├── client.c          # Mã nguồn client
├── account.txt       # File lưu thông tin tài khoản
├── Makefile          # Cấu hình biên dịch
└── README.md         # File này
```

## Tính năng chính

### Server
- **Poll() Multiplexing**: Xử lý nhiều clients đồng thời hiệu quả
- **Quản lý tài khoản**: Đọc/ghi file account.txt
- **Theo dõi đăng nhập sai**: Đếm số lần sai và khóa tài khoản
- **Multi-login**: Cho phép một tài khoản đăng nhập từ nhiều terminal
- **Single account per client**: Mỗi client chỉ đăng nhập một tài khoản

### Client
- **Menu tương tác**: Giao diện đơn giản, dễ sử dụng
- **Đăng nhập/Đăng xuất**: Gửi yêu cầu và nhận phản hồi
- **Hiển thị kết quả**: In rõ ràng thông báo từ server

## Danh sách tài khoản mặc định

| Username     | Password  | Status    |
|--------------|-----------|-----------|
| admin        | admin123  | Unlocked  |
| user1        | pass123   | Unlocked  |
| user2        | pass456   | Unlocked  |
| test         | test      | Unlocked  |
| alice        | alice123  | Unlocked  |
| bob          | bob456    | Unlocked  |
| locked_user  | password  | Locked    |

## Ghi chú quan trọng

- Số lần đăng nhập sai tối đa: **3 lần**
- Sau 3 lần sai, tài khoản tự động bị khóa và ghi vào file
- Một tài khoản có thể đăng nhập từ nhiều client khác nhau
- Mỗi client chỉ được đăng nhập một tài khoản tại một thời điểm
- Server sử dụng `poll()` để quản lý I/O multiplexing
- Tài khoản bị khóa cần được mở khóa thủ công trong file `account.txt`

## Xử lý lỗi

- Tài khoản không tồn tại
- Sai mật khẩu (với thông báo số lần còn lại)
- Tài khoản đã bị khóa
- Client đã đăng nhập (không thể login lần 2)
- Client chưa đăng nhập (không thể logout)
- Mất kết nối với server
