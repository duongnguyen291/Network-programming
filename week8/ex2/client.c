#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 2048

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <ServerIP> <Port>\n", argv[0]);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    char buf[BUF_SIZE];

    while (1) {
        printf("Input: ");
        fgets(buf, BUF_SIZE, stdin);

        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) == 0)
            break;

        sendto(sockfd, buf, strlen(buf), 0,
               (struct sockaddr *)&servaddr,
               sizeof(servaddr));

        memset(buf, 0, BUF_SIZE);
        recvfrom(sockfd, buf, BUF_SIZE - 1, 0, NULL, NULL);

        printf("%s\n", buf);
    }

    close(sockfd);
    return 0;
}
