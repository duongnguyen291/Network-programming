#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define BUFF_SIZE 1024

// Hàm tách chuỗi thành alphabet và digit
int process_string(char* input, char* alphabet, char* digits) {
    int alpha_len = 0;
    int digit_len = 0;
    int has_invalid = 0;
    
    for (int i = 0; input[i] != '\0' && input[i] != '\n'; i++) {
        if (isalpha(input[i])) {
            alphabet[alpha_len++] = input[i];
        } else if (isdigit(input[i])) {
            digits[digit_len++] = input[i];
        } else {
            has_invalid = 1;
        }
    }
    
    alphabet[alpha_len] = '\0';
    digits[digit_len] = '\0';
    
    return has_invalid;
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
    char buff[BUFF_SIZE];
    char alphabet[BUFF_SIZE];
    char digits[BUFF_SIZE];
    char response[BUFF_SIZE * 2];
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
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    // 3. Lắng nghe kết nối
    listen(listenfd, 5);
    printf("String Processing Server started on port %d\n", port);
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

        // 5. Xử lý chuỗi từ client
        while (1) {
            int n = recv(connfd, buff, BUFF_SIZE, 0);
            if (n <= 0) {
                printf("Client disconnected or error occurred\n");
                break;
            }
            
            buff[n] = '\0';
            printf("Received from client: %s", buff);
            
            // Kiểm tra chuỗi rỗng (chỉ có newline)
            if (strlen(buff) <= 1) {
                printf("Client sent empty string, closing connection\n");
                break;
            }
            
            // Xử lý chuỗi
            int has_invalid = process_string(buff, alphabet, digits);
            
            if (has_invalid) {
                // Có ký tự không hợp lệ
                strcpy(response, "Error");
                printf("Error: String contains invalid characters\n");
            } else {
                // Tạo response với 2 chuỗi kết quả
                snprintf(response, sizeof(response), "%s\n%s", digits, alphabet);
                printf("Processed - Digits: '%s', Alphabet: '%s'\n", digits, alphabet);
            }
            
            // Gửi response về client
            send(connfd, response, strlen(response), 0);
            printf("Response sent to client\n");
        }
        
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
