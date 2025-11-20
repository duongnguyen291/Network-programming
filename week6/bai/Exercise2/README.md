# Exercise 2: TCP File Uppercase Server/Client

## Hướng dẫn chạy

### Bước 1: Compile
```bash
cd Exercise2
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
cd Exercise2
./server
```

**Kết quả hiển thị:**
```
Server dang lang nghe tren 127.0.0.1:5501
```

### Bước 3: Chuẩn bị file input
Tạo file văn bản để test, ví dụ:
```bash
echo "Hello World
This is a test file
Line 3 with numbers 123" > input.txt
```

### Bước 4: Chạy Client
Mở terminal thứ hai:
```bash
cd Exercise2
./client
```

**Kết quả hiển thị:**
```
Nhap duong dan file van ban: 
```

Nhập đường dẫn file (ví dụ: `input.txt` hoặc đường dẫn đầy đủ):
```
Nhap duong dan file van ban: input.txt
Da ket noi thanh cong den server 127.0.0.1:5501
```

## Kết quả

### Sau khi xử lý xong, client sẽ hiển thị:
```
File da duoc xu ly. Ket qua luu vao 'output_capitalized.txt'
Tong so byte da gui: [số byte]
```

### File output
File kết quả được lưu với tên `output_capitalized.txt` trong thư mục hiện tại.

**Ví dụ:**

**File input (`input.txt`):**
```
hello world
this is a test
line 3
```

**File output (`output_capitalized.txt`):**
```
HELLO WORLD
THIS IS A TEST
LINE 3
```

## Kết quả mong đợi

| **File Input** | **File Output** |
|---------------|----------------|
| `hello world` | `HELLO WORLD` |
| `Test123` | `TEST123` |
| `Multiple lines`<br>`in file` | `MULTIPLE LINES`<br>`IN FILE` |
| File rỗng | File rỗng |

## Lưu ý

- **Luôn chạy server trước**, sau đó mới chạy client
- Server chạy trên **port 5501** (khác với Exercise 1)
- File output luôn có tên `output_capitalized.txt`
- Để dừng server, nhấn `Ctrl+C` trong terminal chạy server
- Client sẽ hiển thị tổng số byte đã gửi trước khi thoát