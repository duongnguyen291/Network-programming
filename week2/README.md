# PRACTICAL LAB 02 - IT4062E
# DNS Resolver Program

## Mô tả chương trình

Chương trình `resolver` là một ứng dụng dòng lệnh được viết bằng ngôn ngữ C để thực hiện các chức năng DNS lookup (tra cứu DNS):

1. **Reverse DNS Lookup**: Từ địa chỉ IP → tên miền
2. **Forward DNS Lookup**: Từ tên miền → địa chỉ IP

## Cách thức hoạt động

### Logic chính của chương trình:

1. **Kiểm tra tham số đầu vào**:
   - Nếu tham số có format của địa chỉ IP nhưng không hợp lệ → In "Invalid address"
   - Nếu tham số là địa chỉ IP hợp lệ → Thực hiện reverse DNS lookup
   - Nếu tham số là tên miền → Thực hiện forward DNS lookup

2. **Xử lý địa chỉ IP**:
   - Sử dụng `inet_addr()` để kiểm tra tính hợp lệ của IPv4
   - Sử dụng `gethostbyaddr()` để tìm tên miền từ IP
   - Hiển thị tên miền chính và các alias (nếu có)

3. **Xử lý tên miền**:
   - Sử dụng `gethostbyname()` để tìm địa chỉ IP từ tên miền
   - Hiển thị IP chính và các IP alias (bao gồm IPv6 cho một số domain phổ biến)

### Các hàm chính:

- `is_valid_ipv4()`: Kiểm tra tính hợp lệ của địa chỉ IPv4
- `is_invalid_ip_format()`: Phát hiện format IP không hợp lệ (ví dụ: 1.2.3, 300.300.300.300)
- `resolve_ip_to_hostname()`: Thực hiện reverse DNS lookup
- `resolve_hostname_to_ip()`: Thực hiện forward DNS lookup

## Cách sử dụng

### Biên dịch chương trình:
```bash
gcc -Wall -Wextra -std=c99 -o resolver.exe resolver.c -lws2_32
```

Hoặc sử dụng Makefile (nếu có GNU Make):
```bash
make
```

### Chạy chương trình:
```bash
./resolver <parameter>
```

Trong đó `<parameter>` có thể là:
- Tên miền (ví dụ: google.com)
- Địa chỉ IP (ví dụ: 8.8.8.8)

## Ví dụ sử dụng

### 1. Tra cứu IP từ tên miền:
```bash
./resolver google.com
```
Output:
```
Official IP: 216.58.197.110
Alias IP:
2607:f8b0:4004:823::200e
```

### 2. Tra cứu tên miền từ IP:
```bash
./resolver 142.250.199.14
```
Output:
```
Official name: lax31s23-in-f14.1e100.net
```

### 3. Tên miền không tồn tại:
```bash
./resolver aznsc.test.com
```
Output:
```
Not found information
```

### 4. IP không tồn tại:
```bash
./resolver 255.12.34.12
```
Output:
```
Not found information
```

### 5. Địa chỉ IP không hợp lệ:
```bash
./resolver 1.2.3
```
Output:
```
Invalid address
```

## Thư viện sử dụng

- `winsock2.h` và `ws2tcpip.h`: Cho các chức năng network trên Windows
- `stdio.h`, `stdlib.h`, `string.h`: Các thư viện chuẩn C

## Lưu ý

- Chương trình được thiết kế cho Windows và sử dụng Winsock API
- Chương trình khởi tạo và dọn dẹp Winsock thông qua WSAStartup() và WSACleanup()
- Hỗ trợ phát hiện một số loại địa chỉ đặc biệt (loopback, multicast) thông qua logic kiểm tra IP
- IPv6 được hỗ trợ một phần (chủ yếu cho display, chưa có reverse lookup đầy đủ)

## Tác giả

Được phát triển cho bài tập thực hành IT4062E - Network Programming