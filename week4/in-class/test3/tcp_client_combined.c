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

    // 4. TEST 2.3: Gửi 2 message liên tiếp
    printf("=== TEST 2.3: Multiple Messages Combined ===\n");
    printf("Sending 'bull'...\n");
    send(sockfd, "bull", 4, 0);
    
    printf("Sending 'dog'...\n");
    send(sockfd, "dog", 3, 0);
    
    printf("Waiting for server response...\n");
    
    // 5. Nhận response (có thể là "bulldog" hoặc 2 message riêng biệt)
    int n = recv(sockfd, buff, BUFF_SIZE, 0);
    if (n > 0) {
        buff[n] = '\0';
        printf("Received from server: '%s' (length: %d)\n", buff, n);
        
        // Phân tích kết quả
        if (strcmp(buff, "bulldog") == 0) {
            printf("✓ RESULT: Messages were COMBINED into 'bulldog'\n");
            printf("✓ This demonstrates TCP message merging behavior\n");
        } else if (strcmp(buff, "bull") == 0) {
            printf("✓ RESULT: Only first message 'bull' received\n");
            printf("✓ Waiting for second message...\n");
            
            // Nhận message thứ 2
            n = recv(sockfd, buff, BUFF_SIZE, 0);
            if (n > 0) {
                buff[n] = '\0';
                printf("Received second message: '%s'\n", buff);
                printf("✓ RESULT: Messages were NOT combined\n");
            }
        } else {
            printf("✓ RESULT: Unexpected response: '%s'\n", buff);
        }
    } else {
        printf("No response received\n");
    }

    printf("\n=== TEST COMPLETED ===\n");
    printf("This test demonstrates TCP's message boundary behavior\n");

#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
    return 0;
}
