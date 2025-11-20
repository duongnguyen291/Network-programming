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
#define SERVER_PORT 5501
#define BUFFER_CHUNK 4096

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char *file_buffer = NULL;
    size_t buffer_size = 0;
    ssize_t bytes_received;

    // Bo qua SIGCHLD de tranh zombie processes; van dung fork de mo rong neu can
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
            // Tien trinh con xu ly client nay
            close(server_sock);

            file_buffer = NULL;
            buffer_size = 0;

            while (1) {
                char chunk[BUFFER_CHUNK];
                bytes_received = recv(client_sock, chunk, sizeof(chunk), 0);
                if (bytes_received < 0) {
                    perror("recv");
                    free(file_buffer);
                    close(client_sock);
                    exit(EXIT_FAILURE);
                } else if (bytes_received == 0) {
                    // Client da gui xong
                    break;
                }

                char *new_buf = realloc(file_buffer, buffer_size + (size_t)bytes_received);
                if (!new_buf) {
                    perror("realloc");
                    free(file_buffer);
                    close(client_sock);
                    exit(EXIT_FAILURE);
                }
                file_buffer = new_buf;
                memcpy(file_buffer + buffer_size, chunk, (size_t)bytes_received);
                buffer_size += (size_t)bytes_received;
            }

            if (buffer_size == 0) {
                // Khong nhan duoc gi; chi dong ket noi
                free(file_buffer);
                close(client_sock);
                exit(0);
            }

            // Chuyen doi buffer thanh chu hoa
            for (size_t i = 0; i < buffer_size; i++) {
                file_buffer[i] = (char)toupper((unsigned char)file_buffer[i]);
            }

            // Gui lai cho client
            size_t total_sent = 0;
            while (total_sent < buffer_size) {
                ssize_t sent = send(client_sock, file_buffer + total_sent,
                                    buffer_size - total_sent, 0);
                if (sent <= 0) {
                    perror("send");
                    break;
                }
                total_sent += (size_t)sent;
            }

            free(file_buffer);
            close(client_sock);
            exit(0);
        } else {
            // Tien trinh cha
            close(client_sock);
        }
    }

    close(server_sock);
    return 0;
}
