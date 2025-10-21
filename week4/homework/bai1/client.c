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

#define BUFF_SIZE 1024

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
    printf("String Processing Client\n");
    printf("Enter strings to process (empty string to exit):\n");

    // 4. Vòng lặp gửi chuỗi
    while (1) {
        printf("\nEnter string: ");
        fgets(buff, BUFF_SIZE, stdin);
        
        // Kiểm tra chuỗi rỗng (chỉ có newline)
        if (strlen(buff) <= 1) {
            printf("Empty string entered. Closing connection...\n");
            break;
        }
        
        // Gửi chuỗi lên server
        send(sockfd, buff, strlen(buff), 0);
        printf("String sent to server\n");
        
        // Nhận response từ server
        int n = recv(sockfd, buff, BUFF_SIZE, 0);
        if (n <= 0) {
            printf("Server disconnected or error occurred\n");
            break;
        }
        
        buff[n] = '\0';
        
        // Xử lý và hiển thị kết quả
        if (strcmp(buff, "Error") == 0) {
            printf("Server Response: Error\n");
            printf("The string contains invalid characters (non-alphabet and non-digit)\n");
        } else {
            // Tách response thành 2 phần (digits và alphabet)
            char* newline_pos = strchr(buff, '\n');
            if (newline_pos != NULL) {
                *newline_pos = '\0';
                char* digits = buff;
                char* alphabet = newline_pos + 1;
                
                printf("Server Response:\n");
                printf("Digits: %s\n", digits);
                printf("Alphabet: %s\n", alphabet);
            } else {
                printf("Server Response: %s\n", buff);
            }
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
