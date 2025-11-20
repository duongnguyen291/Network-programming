#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5501
#define BUFFER_CHUNK 4096

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char file_path[512];
    FILE *fp_in = NULL;
    FILE *fp_out = NULL;
    char buffer[BUFFER_CHUNK];
    ssize_t bytes_sent, bytes_received;
    long long total_bytes_sent = 0;

    fprintf(stdout, "Nhap duong dan file van ban: ");
    if (fgets(file_path, sizeof(file_path), stdin) == NULL) {
        fprintf(stderr, "Khong the doc duong dan file.\n");
        return EXIT_FAILURE;
    }

    // Xoa ky tu xuong dong o cuoi
    size_t len = strlen(file_path);
    if (len > 0 && file_path[len - 1] == '\n') {
        file_path[len - 1] = '\0';
    }

    fp_in = fopen(file_path, "rb");
    if (!fp_in) {
        perror("fopen input");
        return EXIT_FAILURE;
    }

    fp_out = fopen("output_capitalized.txt", "wb");
    if (!fp_out) {
        perror("fopen output");
        fclose(fp_in);
        return EXIT_FAILURE;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        fclose(fp_in);
        fclose(fp_out);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        fclose(fp_in);
        fclose(fp_out);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        fprintf(stderr, "Khong the ket noi den server %s:%d\n", SERVER_IP, SERVER_PORT);
        close(sock);
        fclose(fp_in);
        fclose(fp_out);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Da ket noi thanh cong den server %s:%d\n", SERVER_IP, SERVER_PORT);

    // Gui noi dung file
    while (1) {
        size_t read_bytes = fread(buffer, 1, sizeof(buffer), fp_in);
        if (read_bytes > 0) {
            size_t offset = 0;
            while (offset < read_bytes) {
                bytes_sent = send(sock, buffer + offset, read_bytes - offset, 0);
                if (bytes_sent <= 0) {
                    perror("send");
                    close(sock);
                    fclose(fp_in);
                    fclose(fp_out);
                    exit(EXIT_FAILURE);
                }
                total_bytes_sent += bytes_sent;
                offset += (size_t)bytes_sent;
            }
        }

        if (read_bytes < sizeof(buffer)) {
            if (feof(fp_in)) {
                break;
            } else {
                perror("fread");
                close(sock);
                fclose(fp_in);
                fclose(fp_out);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Bao hieu ket thuc gui du lieu
    shutdown(sock, SHUT_WR);

    // Nhan file da xu ly
    while (1) {
        bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            perror("recv");
            close(sock);
            fclose(fp_in);
            fclose(fp_out);
            exit(EXIT_FAILURE);
        } else if (bytes_received == 0) {
            // Server dong ket noi
            break;
        }

        size_t written = fwrite(buffer, 1, (size_t)bytes_received, fp_out);
        if (written < (size_t)bytes_received) {
            perror("fwrite");
            close(sock);
            fclose(fp_in);
            fclose(fp_out);
            exit(EXIT_FAILURE);
        }
    }

    fprintf(stdout, "File da duoc xu ly. Ket qua luu vao 'output_capitalized.txt'\n");
    fprintf(stdout, "Tong so byte da gui: %lld\n", total_bytes_sent);

    close(sock);
    fclose(fp_in);
    fclose(fp_out);
    return 0;
}
