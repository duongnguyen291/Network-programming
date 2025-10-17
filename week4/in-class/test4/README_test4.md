# TEST 2.4: FRAGMENTATION WHEN RECEIVER BUFF_SIZE IS SMALL

## 🎯 MỤC TIÊU
Chứng minh partial reads - một lần gửi lớn có thể bị chia thành nhiều lần nhận nhỏ.

## 📝 GIẢI THÍCH HIỆN TƯỢNG

### **TCP Fragmentation Behavior:**
- **Large Message**: Client gửi 1 message lớn (>4KB)
- **Small Buffer**: Server có buffer nhỏ (7 bytes)
- **Partial Reads**: Server nhận được nhiều lần, mỗi lần 7 bytes
- **Fragmentation**: Message lớn bị chia thành nhiều fragment nhỏ

### **Kết quả mong đợi:**
- Client gửi: 1 message lớn (ví dụ: 1000 ký tự)
- Server nhận: Nhiều lần, mỗi lần 7 bytes
- Log: `ret: 7, ret: 7, ret: 7, ..., ret: <7` (lần cuối)

## 🔧 CÁC BƯỚC THỰC HIỆN

### **Phương án 1: Giảm BUFF_SIZE của Server**
- Sửa `#define BUFF_SIZE 1024` thành `#define BUFF_SIZE 7`
- Client gửi message bình thường

### **Phương án 2: Giữ nguyên Server, tăng kích thước Client**
- Giữ `BUFF_SIZE 1024` của server
- Client gửi message rất lớn (>4KB)

## 📊 KẾT QUẢ MONG ĐỢI

### **Server Log:**
```
Server started. Waiting for connection on port 8080...
Client connected: 127.0.0.1:xxxxx
ret: 7
ret: 7
ret: 7
ret: 7
...
ret: 3
```

### **Client Log:**
```
Connected to server!
Sending large message (1000 characters)...
Message sent successfully!
```

## 🔍 GIẢI THÍCH KỸ THUẬT

### **Tại sao xảy ra fragmentation?**

1. **Buffer Size Limitation**: Server buffer nhỏ hơn message size
2. **TCP Segment Size**: TCP có giới hạn segment size (MTU)
3. **OS Socket Buffer**: Hệ điều hành có giới hạn buffer
4. **Network MTU**: Router có giới hạn packet size
5. **Flow Control**: TCP flow control có thể chia nhỏ data

### **Các yếu tố ảnh hưởng:**
- **BUFF_SIZE**: Kích thước buffer của receiver
- **Message Size**: Kích thước message gửi đi
- **MTU**: Maximum Transmission Unit của network
- **TCP Window**: TCP window size
- **OS Limits**: Giới hạn của hệ điều hành

## 📸 EVIDENCE CẦN THU THẬP

1. **Server log**: Hiển thị số bytes nhận được mỗi lần
2. **Client log**: Hiển thị message gửi đi
3. **Timing**: Thời gian giữa các recv()
4. **Buffer content**: Nội dung buffer sau mỗi recv()

## 🎯 KẾT LUẬN

Test này chứng minh rằng:
- **TCP không đảm bảo message boundaries**
- **Large message có thể bị chia thành nhiều fragment**
- **Receiver phải xử lý partial reads**
- **Ứng dụng cần implement message reassembly**

Đây là lý do tại sao các protocol cần:
- **Length prefix**: Để biết message size
- **Delimiter**: Để biết message boundary
- **Framing**: Để reassemble message
- **Buffer management**: Để xử lý partial reads
