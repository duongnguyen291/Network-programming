#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5500
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char send_buffer[BUFFER_SIZE];
    char recv_buffer[BUFFER_SIZE];
    ssize_t bytes_sent, bytes_received;
    long long total_bytes_sent = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        fprintf(stderr, "Khong the ket noi den server %s:%d\n", SERVER_IP, SERVER_PORT);
        close(sock);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Da ket noi thanh cong den server %s:%d\n", SERVER_IP, SERVER_PORT);
    fprintf(stdout, "Nhap tin nhan de gui. Nhap 'q' hoac 'Q' de thoat.\n");

    while (1) {
        fprintf(stdout, "Nhap: ");
        if (fgets(send_buffer, sizeof(send_buffer), stdin) == NULL) {
            break;
        }

        size_t len = strlen(send_buffer);
        if (len == 0) {
            continue;
        }

        // Kiem tra lenh thoat truoc khi gui
        if ((len == 2 && (send_buffer[0] == 'q' || send_buffer[0] == 'Q') && send_buffer[1] == '\n') ||
            (len == 1 && (send_buffer[0] == 'q' || send_buffer[0] == 'Q'))) {
            // Van gui lenh thoat de thong bao cho server
            bytes_sent = send(sock, send_buffer, len, 0);
            if (bytes_sent > 0) {
                total_bytes_sent += bytes_sent;
            }
            break;
        }

        bytes_sent = send(sock, send_buffer, len, 0);
        if (bytes_sent < 0) {
            perror("send");
            break;
        }
        total_bytes_sent += bytes_sent;

        memset(recv_buffer, 0, sizeof(recv_buffer));
        bytes_received = recv(sock, recv_buffer, sizeof(recv_buffer) - 1, 0);
        if (bytes_received <= 0) {
            // Server dong ket noi hoac co loi
            break;
        }

        recv_buffer[bytes_received] = '\0';
        fprintf(stdout, "Nhan tu server: %s", recv_buffer);
    }

    fprintf(stdout, "\nTong so byte da gui: %lld\n", total_bytes_sent);
    close(sock);
    return 0;
}
