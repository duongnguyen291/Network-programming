#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5500
#define BUFFER_SIZE 1024

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            // Client dong ket noi hoac co loi
            break;
        }

        // Loai bo ky tu xuong dong neu co
        if (buffer[bytes_received - 1] == '\n') {
            buffer[bytes_received - 1] = '\0';
            bytes_received--;
        }

        // Kiem tra lenh thoat
        if ((bytes_received == 1) && (buffer[0] == 'q' || buffer[0] == 'Q')) {
            // Khong gui gi ve, chi dong ket noi
            break;
        }

        // Chuyen doi thanh chu hoa
        for (ssize_t i = 0; i < bytes_received; i++) {
            buffer[i] = (char)toupper((unsigned char)buffer[i]);
        }

        // Them ky tu xuong dong de hien thi dep hon
        buffer[bytes_received] = '\n';
        ssize_t bytes_to_send = bytes_received + 1;
        ssize_t total_sent = 0;
        while (total_sent < bytes_to_send) {
            ssize_t sent = send(client_sock, buffer + total_sent, bytes_to_send - total_sent, 0);
            if (sent <= 0) {
                perror("send");
                break;
            }
            total_sent += sent;
        }
    }

    close(client_sock);
    exit(0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;

    // Bo qua SIGCHLD de tranh zombie processes
    signal(SIGCHLD, SIG_IGN);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0) {
        perror("listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server dang lang nghe tren %s:%d\n", SERVER_IP, SERVER_PORT);

    while (1) {
        client_addr_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_sock);
            continue;
        } else if (pid == 0) {
            // Tien trinh con
            close(server_sock);
            handle_client(client_sock);
        } else {
            // Tien trinh cha
            close(client_sock);
        }
    }

    close(server_sock);
    return 0;
}
