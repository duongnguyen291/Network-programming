#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>

#define BUF_SIZE 1024

typedef struct {
    char alpha[BUF_SIZE];
    char digit[BUF_SIZE];
    int undef;
} Result;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <ServerIP> <Port>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char sendbuf[BUF_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    while (1) {
        printf("Input string: ");
        fgets(sendbuf, BUF_SIZE, stdin);

        if (strcmp(sendbuf, "\n") == 0)
            break;

        send(sockfd, sendbuf, strlen(sendbuf), 0);

        Result res;
        struct iovec iov;

        iov.iov_base = &res;
        iov.iov_len  = sizeof(res);

        readv(sockfd, &iov, 1);

        printf("Alphabet: %s\n", res.alpha);
        printf("Digit: %s\n", res.digit);
        if (res.undef > 0)
            printf("There is %d undefined character(s)\n", res.undef);
    }

    close(sockfd);
    return 0;
}
