# Exercise 1: TCP Message Uppercase Server/Client

## Hướng dẫn chạy

### Bước 1: Compile
```bash
cd Exercise1
make
```

Hoặc compile thủ công:
```bash
gcc -Wall -Wextra -std=c11 -o server server.c
gcc -Wall -Wextra -std=c11 -o client client.c
```

### Bước 2: Chạy Server
Mở terminal thứ nhất:
```bash
cd Exercise1
./server
```

**Kết quả hiển thị:**
```
Server dang lang nghe tren 127.0.0.1:5500
```

### Bước 3: Chạy Client
Mở terminal thứ hai:
```bash
cd Exercise1
./client
```

**Kết quả hiển thị:**
```
Da ket noi thanh cong den server 127.0.0.1:5500
Nhap tin nhan de gui. Nhap 'q' hoac 'Q' de thoat.
Nhap: 
```

## Cách sử dụng

### Gửi tin nhắn
Nhập tin nhắn bất kỳ và nhấn Enter. Server sẽ chuyển thành chữ hoa và gửi lại.

**Ví dụ:**
```
Nhap: hello world
Nhan tu server: HELLO WORLD

Nhap: Test Message 123
Nhan tu server: TEST MESSAGE 123
```

### Thoát chương trình
Nhập `q` hoặc `Q` và nhấn Enter để thoát.

**Kết quả:**
```
Nhap: q

Tong so byte da gui: 2
```

## Kết quả mong đợi

| **Input từ Client** | **Output từ Server** |
|---------------------|---------------------|
| `hello` | `HELLO` |
| `Hello World` | `HELLO WORLD` |
| `test123` | `TEST123` |
| `q` | (Đóng kết nối) |

## Lưu ý

- **Luôn chạy server trước**, sau đó mới chạy client
- Server chạy trên **port 5500**
- Để dừng server, nhấn `Ctrl+C` trong terminal chạy server
- Client sẽ hiển thị tổng số byte đã gửi trước khi thoát