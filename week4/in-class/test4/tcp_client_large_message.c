#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define SERV_PORT 8080
#define BUFF_SIZE 1024

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }
#endif

    int sockfd;
    char buff[BUFF_SIZE];
    struct sockaddr_in servaddr;

    // 1. Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket() failed");
        exit(1);
    }

    // 2. Khai báo địa chỉ server
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
#ifdef _WIN32
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
#endif

    // 3. Kết nối tới server
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect() failed");
        exit(1);
    }
    printf("Connected to server!\n");

    // 4. TEST 2.4: Gửi message lớn
    printf("=== TEST 2.4: Large Message Fragmentation ===\n");
    
    // Tạo message lớn (1000 ký tự)
    char large_message[1001];
    for (int i = 0; i < 1000; i++) {
        large_message[i] = 'A' + (i % 26);  // A-Z repeating
    }
    large_message[1000] = '\n';  // Add newline at end
    
    printf("Sending large message (1000 characters + newline)...\n");
    printf("Message preview: %.50s...\n", large_message);
    
    // Gửi message lớn
    int bytes_sent = send(sockfd, large_message, 1001, 0);
    printf("Sent %d bytes to server\n", bytes_sent);
    
    // Nhận echo từ server (có thể bị chia thành nhiều fragment)
    printf("Waiting for server echo...\n");
    
    int total_received = 0;
    int recv_count = 0;
    
    while (total_received < 1001) {
        int n = recv(sockfd, buff, BUFF_SIZE, 0);
        if (n <= 0) {
            printf("Connection closed or error occurred\n");
            break;
        }
        
        recv_count++;
        total_received += n;
        buff[n] = '\0';
        
        printf("Received fragment #%d: %d bytes (total: %d/%d)\n", 
               recv_count, n, total_received, 1001);
        printf("Fragment content: '%.*s'\n", n, buff);
        
        // Check if we received the complete message
        if (strchr(buff, '\n') != NULL) {
            printf("Complete message received!\n");
            break;
        }
    }
    
    printf("=== TEST COMPLETED ===\n");
    printf("Total fragments received: %d\n", recv_count);
    printf("Total bytes received: %d\n", total_received);
    printf("This demonstrates TCP message fragmentation behavior\n");

#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
    return 0;
}
