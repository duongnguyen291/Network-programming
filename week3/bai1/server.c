#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib")

static void print_client_info(const char* action, const struct sockaddr_in* client) {
    char* ip_str = inet_ntoa(client->sin_addr);
    printf("[%s] %s:%d\n", action, ip_str, ntohs(client->sin_port));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        return 1;
    }

    setvbuf(stdout, NULL, _IONBF, 0);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((u_short)atoi(argv[1]));

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Server listening on 0.0.0.0:%s\n", argv[1]);

    char buffer[2048];
    while (1) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        int bytes_received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, 
                                     (struct sockaddr*)&client_addr, &client_len);
        if (bytes_received <= 0) continue;
        buffer[bytes_received] = '\0';

        print_client_info("RECV", &client_addr);
        printf("  payload=\"%s\"\n", buffer);

        if (buffer[0] == '\0') continue;

        int has_invalid_char = 0;
        char digits[2048] = {0};
        char letters[2048] = {0};
        int digit_count = 0, letter_count = 0;

        for (int i = 0; i < bytes_received; ++i) {
            unsigned char c = (unsigned char)buffer[i];
            if (isdigit(c)) {
                digits[digit_count++] = (char)c;
            } else if (isalpha(c)) {
                letters[letter_count++] = (char)c;
            } else {
                has_invalid_char = 1;
            }
        }
        digits[digit_count] = '\0';
        letters[letter_count] = '\0';

        if (has_invalid_char) {
            const char* error_msg = "Error";
            sendto(sock, error_msg, (int)strlen(error_msg), 0, 
                   (struct sockaddr*)&client_addr, client_len);
            printf("SEND: Error\n");
        } else {
            char response[4096];
            snprintf(response, sizeof(response), "%s\n%s", digits, letters);
            sendto(sock, response, (int)strlen(response), 0, 
                   (struct sockaddr*)&client_addr, client_len);
            printf("SEND:\n%s\n", response);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
