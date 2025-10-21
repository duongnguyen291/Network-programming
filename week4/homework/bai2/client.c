#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define BUFF_SIZE 1024
#define MAX_FILENAME 256

// Kiểm tra file tồn tại và lấy kích thước
long get_file_size(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return -1; // File không tồn tại
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

// Gửi file lên server
int send_file(int sockfd, const char* filepath) {
    char filename[MAX_FILENAME];
    char buff[BUFF_SIZE];
    char response[BUFF_SIZE];
    FILE* file;
    long file_size;
    long bytes_sent = 0;
    int n;
    
    // Lấy tên file từ đường dẫn
    const char* last_slash = strrchr(filepath, '/');
    if (last_slash == NULL) {
        last_slash = strrchr(filepath, '\\'); // Windows path
    }
    
    if (last_slash != NULL) {
        strcpy(filename, last_slash + 1);
    } else {
        strcpy(filename, filepath);
    }
    
    // Loại bỏ newline nếu có
    char* newline = strchr(filename, '\n');
    if (newline) *newline = '\0';
    
    printf("Sending file: %s\n", filename);
    
    // Kiểm tra file tồn tại
    file_size = get_file_size(filepath);
    if (file_size < 0) {
        printf("Error: File not found\n");
        return 0;
    }
    
    if (file_size > 100 * 1024 * 1024) { // Max 100MB
        printf("Error: File too large (max 100MB)\n");
        return 0;
    }
    
    printf("File size: %ld bytes\n", file_size);
    
    // Gửi tên file
    send(sockfd, filename, strlen(filename), 0);
    
    // Nhận response từ server
    n = recv(sockfd, response, BUFF_SIZE, 0);
    if (n <= 0) {
        printf("Error: Server disconnected\n");
        return 0;
    }
    response[n] = '\0';
    
    if (strcmp(response, "OK") != 0) {
        printf("Server Response: %s\n", response);
        return 0;
    }
    
    // Gửi kích thước file
    snprintf(buff, sizeof(buff), "%ld", file_size);
    send(sockfd, buff, strlen(buff), 0);
    
    // Mở file để đọc
    file = fopen(filepath, "rb");
    if (file == NULL) {
        printf("Error: Cannot open file for reading\n");
        return 0;
    }
    
    // Gửi dữ liệu file
    printf("Sending file data...\n");
    while (bytes_sent < file_size) {
        n = fread(buff, 1, BUFF_SIZE, file);
        if (n <= 0) {
            printf("Error: Failed to read file\n");
            fclose(file);
            return 0;
        }
        
        if (send(sockfd, buff, n, 0) <= 0) {
            printf("Error: Failed to send data\n");
            fclose(file);
            return 0;
        }
        
        bytes_sent += n;
        
        // Hiển thị tiến trình
        if (bytes_sent % (1024 * 1024) == 0) { // Mỗi 1MB
            printf("Sent: %ld/%ld bytes (%.1f%%)\n", 
                   bytes_sent, file_size, 
                   (double)bytes_sent / file_size * 100);
        }
    }
    
    fclose(file);
    
    // Nhận response cuối từ server
    n = recv(sockfd, response, BUFF_SIZE, 0);
    if (n <= 0) {
        printf("Error: Server disconnected during transfer\n");
        return 0;
    }
    response[n] = '\0';
    
    printf("Server Response: %s\n", response);
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port_number>\n", argv[0]);
        printf("Example: %s 127.0.0.1 5500\n", argv[0]);
        exit(1);
    }
    
    char* server_ip = argv[1];
    int port = atoi(argv[2]);
    
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

    int sockfd;
    char filepath[MAX_FILENAME];
    struct sockaddr_in servaddr;

    // 1. Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket() failed");
        exit(1);
    }

    // 2. Khai báo địa chỉ server
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
#ifdef _WIN32
    servaddr.sin_addr.s_addr = inet_addr(server_ip);
#else
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);
#endif

    // 3. Kết nối tới server
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect() failed");
        exit(1);
    }
    
    printf("Connected to server %s:%d\n", server_ip, port);
    printf("File Transfer Client\n");
    printf("Enter file paths to transfer (empty path to exit):\n");

    // 4. Vòng lặp gửi file
    while (1) {
        printf("\nEnter file path: ");
        fgets(filepath, MAX_FILENAME, stdin);
        
        // Loại bỏ newline
        char* newline = strchr(filepath, '\n');
        if (newline) *newline = '\0';
        
        // Kiểm tra chuỗi rỗng
        if (strlen(filepath) == 0) {
            printf("Empty path entered. Closing connection...\n");
            break;
        }
        
        // Gửi file
        if (send_file(sockfd, filepath)) {
            printf("File transfer completed successfully!\n");
        } else {
            printf("File transfer failed!\n");
        }
    }

    printf("Disconnecting from server...\n");
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
    return 0;
}
