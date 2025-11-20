#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    if (argc != 3) { 
        fprintf(stderr, "Usage: %s IPAddress PortNumber\n", argv[0]); 
        return 1; 
    }

    WSADATA wsa; 
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) { 
        perror("socket"); 
        WSACleanup(); 
        return 1; 
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((u_short)atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        fprintf(stderr, "Invalid server IP\n"); 
        closesocket(sock); 
        WSACleanup(); 
        return 1;
    }

    char input_line[2048];
    printf("Connected to server %s:%s\n", argv[1], argv[2]);
    
    while (1) {
        printf("Enter string (blank or *** to quit): ");
        if (!fgets(input_line, sizeof(input_line), stdin)) break;
        input_line[strcspn(input_line, "\r\n")] = 0;

        if (input_line[0] == '\0' || strcmp(input_line, "***") == 0) {
            printf("Client closing connection.\n");
            break;
        }

        sendto(sock, input_line, (int)strlen(input_line), 0, 
               (struct sockaddr*)&server_addr, sizeof(server_addr));

        struct sockaddr_in from_addr; 
        int from_len = sizeof(from_addr);
        char response[4096];
        int bytes_received = recvfrom(sock, response, sizeof(response)-1, 0, 
                                     (struct sockaddr*)&from_addr, &from_len);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("Result from server:\n%s\n", response);
        }
    }

    closesocket(sock); 
    WSACleanup(); 
    return 0;
}
