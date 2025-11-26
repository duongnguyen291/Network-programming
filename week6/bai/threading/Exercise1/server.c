#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5500
#define BUFFER_SIZE 1024

// Structure to pass client socket to thread
typedef struct {
    int client_sock;
    struct sockaddr_in client_addr;
} client_info_t;

// Thread function to handle client
void *handle_client(void *arg) {
    client_info_t *client_info = (client_info_t *)arg;
    int client_sock = client_info->client_sock;
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    
    inet_ntop(AF_INET, &(client_info->client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    client_port = ntohs(client_info->client_addr.sin_port);
    printf("Client connected from %s:%d\n", client_ip, client_port);
    
    free(client_info); // Free allocated memory
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            // Client closed connection or error occurred
            break;
        }

        // Remove newline character if present
        if (buffer[bytes_received - 1] == '\n') {
            buffer[bytes_received - 1] = '\0';
            bytes_received--;
        }

        // Check for quit command
        if ((bytes_received == 1) && (buffer[0] == 'q' || buffer[0] == 'Q')) {
            // Don't send anything back, just close connection
            break;
        }

        // Convert to uppercase
        for (ssize_t i = 0; i < bytes_received; i++) {
            buffer[i] = (char)toupper((unsigned char)buffer[i]);
        }

        // Add newline for better display
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

    printf("Client %s:%d disconnected\n", client_ip, client_port);
    close(client_sock);
    pthread_exit(NULL);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    pthread_t thread_id;

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

    printf("Server is listening on %s:%d\n", SERVER_IP, SERVER_PORT);
    printf("Using threading technique for handling multiple clients\n");

    while (1) {
        client_addr_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        // Allocate memory for client info
        client_info_t *client_info = malloc(sizeof(client_info_t));
        if (client_info == NULL) {
            perror("malloc");
            close(client_sock);
            continue;
        }
        
        client_info->client_sock = client_sock;
        client_info->client_addr = client_addr;

        // Create a new thread to handle the client
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client_info) != 0) {
            perror("pthread_create");
            free(client_info);
            close(client_sock);
            continue;
        }

        // Detach thread so it cleans up automatically when done
        pthread_detach(thread_id);
    }

    close(server_sock);
    return 0;
}
