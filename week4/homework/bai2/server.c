#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <direct.h>
    #pragma comment(lib, "ws2_32.lib")
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/types.h>
#endif

#define BUFF_SIZE 1024
#define MAX_FILENAME 256

// Tạo thư mục lưu file nếu chưa tồn tại
void create_upload_directory() {
    if (mkdir("uploads", 0755) != 0) {
        // Thư mục đã tồn tại hoặc lỗi khác
        printf("Upload directory: uploads/\n");
    } else {
        printf("Created upload directory: uploads/\n");
    }
}

// Kiểm tra file đã tồn tại chưa
int file_exists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return 1; // File tồn tại
    }
    return 0; // File không tồn tại
}

// Gửi response về client
void send_response(int connfd, const char* message) {
    send(connfd, message, strlen(message), 0);
    printf("Response sent: %s\n", message);
}

// Xử lý file transfer
void handle_file_transfer(int connfd) {
    char filename[MAX_FILENAME];
    char filepath[MAX_FILENAME + 20];
    char response[BUFF_SIZE];
    char buff[BUFF_SIZE];
    FILE* file;
    long file_size;
    long bytes_received = 0;
    int n;
    
    // Nhận tên file từ client
    n = recv(connfd, filename, MAX_FILENAME, 0);
    if (n <= 0) {
        send_response(connfd, "Error: Failed to receive filename");
        return;
    }
    filename[n] = '\0';
    
    // Loại bỏ newline nếu có
    char* newline = strchr(filename, '\n');
    if (newline) *newline = '\0';
    
    printf("Received filename: %s\n", filename);
    
    // Tạo đường dẫn file đầy đủ
    snprintf(filepath, sizeof(filepath), "uploads/%s", filename);
    
    // Kiểm tra file đã tồn tại
    if (file_exists(filepath)) {
        send_response(connfd, "Error: File already exists on server");
        return;
    }
    
    // Gửi ACK để client biết có thể gửi file
    send_response(connfd, "OK");
    
    // Nhận kích thước file
    n = recv(connfd, buff, BUFF_SIZE, 0);
    if (n <= 0) {
        send_response(connfd, "Error: Failed to receive file size");
        return;
    }
    buff[n] = '\0';
    file_size = atol(buff);
    
    if (file_size <= 0 || file_size > 100 * 1024 * 1024) { // Max 100MB
        send_response(connfd, "Error: Invalid file size");
        return;
    }
    
    printf("File size: %ld bytes\n", file_size);
    
    // Mở file để ghi
    file = fopen(filepath, "wb");
    if (file == NULL) {
        send_response(connfd, "Error: Cannot create file on server");
        return;
    }
    
    // Nhận dữ liệu file
    printf("Receiving file data...\n");
    while (bytes_received < file_size) {
        n = recv(connfd, buff, BUFF_SIZE, 0);
        if (n <= 0) {
            fclose(file);
            remove(filepath); // Xóa file không hoàn chỉnh
            send_response(connfd, "Error: File transfer interrupted");
            return;
        }
        
        fwrite(buff, 1, n, file);
        bytes_received += n;
        
        // Hiển thị tiến trình
        if (bytes_received % (1024 * 1024) == 0) { // Mỗi 1MB
            printf("Received: %ld/%ld bytes (%.1f%%)\n", 
                   bytes_received, file_size, 
                   (double)bytes_received / file_size * 100);
        }
    }
    
    fclose(file);
    printf("File transfer completed: %s (%ld bytes)\n", filename, bytes_received);
    send_response(connfd, "Transfer complete");
}

// Xử lý session với client (có thể gửi nhiều file)
void handle_client_session(int connfd) {
    char filename[MAX_FILENAME];
    char filepath[MAX_FILENAME + 20];
    char response[BUFF_SIZE];
    char buff[BUFF_SIZE];
    FILE* file;
    long file_size;
    long bytes_received = 0;
    int n;
    
    printf("Client session started\n");
    
    // Vòng lặp xử lý nhiều file từ cùng 1 client
    while (1) {
        // Nhận tên file từ client
        n = recv(connfd, filename, MAX_FILENAME, 0);
        if (n <= 0) {
            printf("Client disconnected or error occurred\n");
            break;
        }
        filename[n] = '\0';
        
        // Loại bỏ newline nếu có
        char* newline = strchr(filename, '\n');
        if (newline) *newline = '\0';
        
        // Kiểm tra chuỗi rỗng (client muốn kết thúc)
        if (strlen(filename) == 0) {
            printf("Client sent empty filename, ending session\n");
            break;
        }
        
        printf("Received filename: %s\n", filename);
        
        // Tạo đường dẫn file đầy đủ
        snprintf(filepath, sizeof(filepath), "uploads/%s", filename);
        
        // Kiểm tra file đã tồn tại
        if (file_exists(filepath)) {
            send_response(connfd, "Error: File already exists on server");
            continue; // Tiếp tục với file tiếp theo
        }
        
        // Gửi ACK để client biết có thể gửi file
        send_response(connfd, "OK");
        
        // Nhận kích thước file
        n = recv(connfd, buff, BUFF_SIZE, 0);
        if (n <= 0) {
            send_response(connfd, "Error: Failed to receive file size");
            break;
        }
        buff[n] = '\0';
        file_size = atol(buff);
        
        if (file_size <= 0 || file_size > 100 * 1024 * 1024) { // Max 100MB
            send_response(connfd, "Error: Invalid file size");
            continue;
        }
        
        printf("File size: %ld bytes\n", file_size);
        
        // Mở file để ghi
        file = fopen(filepath, "wb");
        if (file == NULL) {
            send_response(connfd, "Error: Cannot create file on server");
            continue;
        }
        
        // Nhận dữ liệu file
        printf("Receiving file data...\n");
        bytes_received = 0;
        while (bytes_received < file_size) {
            n = recv(connfd, buff, BUFF_SIZE, 0);
            if (n <= 0) {
                fclose(file);
                remove(filepath); // Xóa file không hoàn chỉnh
                send_response(connfd, "Error: File transfer interrupted");
                break;
            }
            
            fwrite(buff, 1, n, file);
            bytes_received += n;
            
            // Hiển thị tiến trình
            if (bytes_received % (1024 * 1024) == 0) { // Mỗi 1MB
                printf("Received: %ld/%ld bytes (%.1f%%)\n", 
                       bytes_received, file_size, 
                       (double)bytes_received / file_size * 100);
            }
        }
        
        fclose(file);
        printf("File transfer completed: %s (%ld bytes)\n", filename, bytes_received);
        send_response(connfd, "Transfer complete");
    }
    
    printf("Client session ended\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port_number>\n", argv[0]);
        printf("Example: %s 5500\n", argv[0]);
        exit(1);
    }
    
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        printf("Error: Invalid port number. Port must be between 1 and 65535\n");
        exit(1);
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }
#endif

    int listenfd, connfd;
    struct sockaddr_in servaddr, clientaddr;
    socklen_t len = sizeof(clientaddr);

    // Tạo thư mục upload
    create_upload_directory();

    // 1. Tạo socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket() failed");
        exit(1);
    }

    // 2. Gán địa chỉ IP và Port
#ifdef _WIN32
    memset(&servaddr, 0, sizeof(servaddr));
#else
    bzero(&servaddr, sizeof(servaddr));
#endif
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    // 3. Lắng nghe kết nối
    listen(listenfd, 5);
    printf("File Transfer Server started on port %d\n", port);
    printf("Upload directory: uploads/\n");
    printf("Waiting for connections...\n");

    // 4. Vòng lặp chấp nhận nhiều client
    while (1) {
        printf("\nWaiting for client connection...\n");
        connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
        if (connfd < 0) {
            perror("accept() failed");
            continue;
        }
        
        printf("Client connected: %s:%d\n", 
               inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 5. Xử lý session với client (có thể gửi nhiều file)
        handle_client_session(connfd);
        
        printf("Client session ended\n");
#ifdef _WIN32
        closesocket(connfd);
#else
        close(connfd);
#endif
    }

#ifdef _WIN32
    closesocket(listenfd);
    WSACleanup();
#else
    close(listenfd);
#endif
    return 0;
}
