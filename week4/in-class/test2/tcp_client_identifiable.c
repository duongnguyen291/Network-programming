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

int main(int argc, char *argv[]) {
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
    
    // Get client ID from command line argument
    char client_id[50] = "Client";
    if (argc > 1) {
        strcpy(client_id, argv[1]);
    }

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
    printf("[%s] Attempting to connect to server...\n", client_id);
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect() failed");
        exit(1);
    }
    printf("[%s] Connected to server!\n", client_id);

    // 4. Gửi và nhận dữ liệu
    for (int i = 0; i < 3; i++) {
        printf("[%s] Enter message %d: ", client_id, i + 1);
        fgets(buff, BUFF_SIZE, stdin);
        
        // Add client ID to message
        char full_message[BUFF_SIZE];
        snprintf(full_message, sizeof(full_message), "[%s] %s", client_id, buff);
        
        send(sockfd, full_message, strlen(full_message), 0);
        int n = recv(sockfd, buff, BUFF_SIZE, 0);
        buff[n] = '\0';
        printf("[%s] Echo from server: %s\n", client_id, buff);
    }

    printf("[%s] Disconnecting...\n", client_id);
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
    return 0;
}
