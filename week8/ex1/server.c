#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

typedef struct {
    char alpha[BUF_SIZE];
    char digit[BUF_SIZE];
    int undef;
} Result;

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <Port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buf[BUF_SIZE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(listenfd);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 5);

    printf("TCP Server listening on port %d\n", port);

    while (1) {
        connfd = accept(listenfd, NULL, NULL);
        if (connfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100000);
                continue;
            }
            perror("accept");
            continue;
        }

        set_nonblocking(connfd);
        printf("Client connected\n");

        while (1) {
            memset(buf, 0, BUF_SIZE);
            int n = recv(connfd, buf, BUF_SIZE, 0);

            if (n > 0) {
                Result res;
                memset(&res, 0, sizeof(res));

                for (int i = 0; buf[i]; i++) {
                    if (isalpha(buf[i]))
                        strncat(res.alpha, &buf[i], 1);
                    else if (isdigit(buf[i]))
                        strncat(res.digit, &buf[i], 1);
                    else if (buf[i] != '\n')
                        res.undef++;
                }

                send(connfd, &res, sizeof(res), 0);
            }
            else if (n == 0) {
                printf("Client disconnected\n");
                close(connfd);
                break;
            }
            else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    usleep(100000);
                    continue;
                }
                perror("recv");
                close(connfd);
                break;
            }
        }
    }
}
