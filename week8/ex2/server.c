#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define BUF_SIZE 2048   // tăng buffer để tránh truncation

int sockfd;
struct sockaddr_in cliaddr;
socklen_t len = sizeof(cliaddr);
char buf[BUF_SIZE];

void sigio_handler(int signo) {
    (void)signo;  // tránh warning unused

    memset(buf, 0, BUF_SIZE);

    int n = recvfrom(sockfd, buf, BUF_SIZE - 1, 0,
                     (struct sockaddr *)&cliaddr, &len);
    if (n <= 0)
        return;

    buf[n] = '\0';

    char result[BUF_SIZE];
    memset(result, 0, BUF_SIZE);

    struct sockaddr_in sa;

    /* ===============================
       CASE 1: input là IP address
       =============================== */
    if (inet_pton(AF_INET, buf, &sa.sin_addr) == 1) {

        char host[NI_MAXHOST];

        if (getnameinfo((struct sockaddr *)&sa, sizeof(sa),
                        host, sizeof(host),
                        NULL, 0, NI_NAMEREQD) == 0) {

            snprintf(result, BUF_SIZE,
                     "Official name: %.900s\nAlias name:\n",
                     host);
        } else {
            snprintf(result, BUF_SIZE,
                     "Not found information\n");
        }
    }

    /* ===============================
       CASE 2: input KHÔNG PHẢI IP
       =============================== */
    else {
        struct addrinfo hints, *res = NULL;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;

        int ret = getaddrinfo(buf, NULL, &hints, &res);

        if (ret == 0 && res != NULL) {
            struct sockaddr_in *addr =
                (struct sockaddr_in *)res->ai_addr;

            snprintf(result, BUF_SIZE,
                     "Official IP: %s\n",
                     inet_ntoa(addr->sin_addr));

            freeaddrinfo(res);
        }
        else {
            /* kiểm tra IP sai format (ví dụ 259.12.34.12) */
            struct sockaddr_in tmp;
            if (inet_pton(AF_INET, buf, &tmp.sin_addr) == 0) {
                snprintf(result, BUF_SIZE,
                         "IP Address is invalid\n");
            } else {
                snprintf(result, BUF_SIZE,
                         "Not found information\n");
            }
        }
    }

    sendto(sockfd, result, strlen(result), 0,
           (struct sockaddr *)&cliaddr, len);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <Port>\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0) {
        perror("bind");
        return 1;
    }

    /* Enable SIGIO */
    fcntl(sockfd, F_SETOWN, getpid());
    int flags = fcntl(sockfd, F_GETFL);
    fcntl(sockfd, F_SETFL, flags | O_ASYNC);

    signal(SIGIO, sigio_handler);

    printf("UDP Server running on port %s...\n", argv[1]);

    while (1)
        pause();

    return 0;
}
