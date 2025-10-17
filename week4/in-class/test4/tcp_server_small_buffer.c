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
#define BUFF_SIZE 7  // Giảm buffer size để test fragmentation

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }
#endif

    int listenfd, connfd;
    char buff[BUFF_SIZE];
    struct sockaddr_in servaddr, clientaddr;
    socklen_t len = sizeof(clientaddr);

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
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    // 3. Lắng nghe kết nối
    listen(listenfd, 5);
    printf("Server started. Waiting for connection on port %d...\n", SERV_PORT);
    printf("BUFF_SIZE = %d bytes (small buffer for fragmentation test)\n", BUFF_SIZE);

    // 4. Chấp nhận kết nối
    connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
    printf("Client connected: %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    // 5. TEST 2.4: Nhận message với buffer nhỏ
    printf("=== TEST 2.4: Fragmentation with Small Buffer ===\n");
    printf("Waiting for large message...\n");
    
    int total_received = 0;
    int recv_count = 0;
    
    while (1) {
        int n = recv(connfd, buff, BUFF_SIZE, 0);
        if (n <= 0) {
            printf("Connection closed or error occurred\n");
            break;
        }
        
        recv_count++;
        total_received += n;
        buff[n] = '\0';  // Null terminate for printing
        
        printf("ret: %d (recv #%d, total: %d bytes)\n", n, recv_count, total_received);
        printf("Received data: '%.*s'\n", n, buff);
        
        // Echo back to client
        send(connfd, buff, n, 0);
        
        // Check if we received a complete message (look for newline or specific pattern)
        if (strchr(buff, '\n') != NULL) {
            printf("Complete message received!\n");
            break;
        }
    }
    
    printf("=== TEST COMPLETED ===\n");
    printf("Total fragments received: %d\n", recv_count);
    printf("Total bytes received: %d\n", total_received);
    printf("This demonstrates TCP message fragmentation with small buffer\n");

#ifdef _WIN32
    closesocket(connfd);
    closesocket(listenfd);
    WSACleanup();
#else
    close(connfd);
    close(listenfd);
#endif
    return 0;
}
