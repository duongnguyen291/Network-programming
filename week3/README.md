# Week 3 - Network Programming Lab

## Tổng quan
Folder này chứa 2 bài tập UDP application:
- **bai1/**: UDP String Processing (Tách ký tự số và chữ)
- **bai2/**: UDP DNS Resolver (Phân giải tên miền)

## Cấu trúc thư mục
```
week3/
├── bai1/          # Bài 1: String Processing
│   ├── client.c
│   ├── server.c
│   ├── Makefile
│   ├── client.exe
│   └── server.exe
├── bai2/          # Bài 2: DNS Resolver
│   ├── client.c
│   ├── server.c
│   ├── Makefile
│   ├── client.exe
│   └── server.exe
└── README.md      # File này
```

---

## Bài 1: UDP String Processing

### Mô tả
Server nhận string từ client, tách ký tự số và chữ cái, trả về kết quả. Nếu có ký tự không hợp lệ thì trả về "Error".

### Cách chạy

#### 1. Compile (nếu chưa có .exe)
```bash
cd week3/bai1
gcc -Wall -Wextra -std=c99 -o server.exe server.c -lws2_32
gcc -Wall -Wextra -std=c99 -o client.exe client.c -lws2_32
```

#### 2. Chạy Server
```bash
cd week3/bai1
./server.exe 5500
```

#### 3. Chạy Client (terminal khác)
```bash
cd week3/bai1
./client.exe 127.0.0.1 5500
```

### Test Cases

| **INPUT** | **EXPECTED OUTPUT** |
|-----------|-------------------|
| `1a2b3cd` | `123`<br>`abcd` |
| `123` | `123`<br>`` (empty letters) |
| `abcd` | `` (empty digits)<br>`abcd` |
| `Ab15CD$` | `Error` |

### Cách test nhanh
```bash
# Test case 1
echo "1a2b3cd" | ./client.exe 127.0.0.1 5500

# Test case 2  
echo "123" | ./client.exe 127.0.0.1 5500

# Test case 3
echo "abcd" | ./client.exe 127.0.0.1 5500

# Test case 4
echo "Ab15CD$" | ./client.exe 127.0.0.1 5500
```

---

## Bài 2: UDP DNS Resolver

### Mô tả
Server nhận domain name hoặc IP address từ client, thực hiện DNS lookup và trả về kết quả.

### Cách chạy

#### 1. Compile (nếu chưa có .exe)
```bash
cd week3/bai2
gcc -Wall -Wextra -std=c99 -o server.exe server.c -lws2_32
gcc -Wall -Wextra -std=c99 -o client.exe client.c -lws2_32
```

#### 2. Chạy Server
```bash
cd week3/bai2
./server.exe 5501
```

#### 3. Chạy Client (terminal khác)
```bash
cd week3/bai2
./client.exe 127.0.0.1 5501
```

### Test Cases

| **INPUT** | **EXPECTED OUTPUT** |
|-----------|-------------------|
| `google.com` | `Official IP: [IP]`<br>`Alias IP:`<br>`[additional IPs]` |
| `8.8.8.8` | `Official name: [hostname]`<br>`Alias name:`<br>`[aliases]` |
| `nonexistent.domain` | `Not found information` |
| `259.12.34.12` | `IP Address is invalid` |

### Cách test nhanh
```bash
# Test domain lookup
echo "google.com" | ./client.exe 127.0.0.1 5501

# Test reverse lookup
echo "8.8.8.8" | ./client.exe 127.0.0.1 5501

# Test invalid domain
echo "nonexistent.test" | ./client.exe 127.0.0.1 5501

# Test invalid IP
echo "259.12.34.12" | ./client.exe 127.0.0.1 5501
```

---

## Lưu ý quan trọng

### 1. Port khác nhau
- **Bài 1**: Sử dụng port 5500
- **Bài 2**: Sử dụng port 5501
- Đảm bảo không chạy 2 server cùng port!

### 2. Thứ tự chạy
1. **Luôn chạy Server trước**
2. Sau đó mới chạy Client
3. Server sẽ chạy liên tục, Client có thể chạy nhiều lần

### 3. Dừng Server
- Nhấn `Ctrl+C` để dừng server
- Hoặc đóng terminal chạy server

### 4. Troubleshooting

#### Lỗi "bind() failed: 10048"
```bash
# Kiểm tra port đang được sử dụng
netstat -ano | findstr :5500
netstat -ano | findstr :5501

# Kill process nếu cần
taskkill /f /pid [PID_NUMBER]
```

#### Lỗi "Invalid server IP"
- Đảm bảo server đã chạy trước
- Kiểm tra IP address (127.0.0.1)
- Kiểm tra port number

### 5. Clean up
```bash
# Xóa file executable
cd week3/bai1
rm -f *.exe *.obj

cd ../bai2  
rm -f *.exe *.obj
```

---

## Kết quả mong đợi

### Bài 1 - String Processing
- ✅ Tách đúng ký tự số và chữ cái
- ✅ Trả về "Error" cho ký tự không hợp lệ
- ✅ Format output đúng

### Bài 2 - DNS Resolver  
- ✅ Domain → IP addresses
- ✅ IP → Domain names
- ✅ "Not found information" cho domain/IP không tồn tại
- ✅ "IP Address is invalid" cho IP không hợp lệ

---

## Yêu cầu hệ thống
- Windows với MinGW hoặc Visual Studio
- Winsock2 library
- Internet connection (cho bài 2 - DNS lookup)
