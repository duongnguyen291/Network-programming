#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 4096

void send_message(int sock, const char *message) {
    send(sock, message, strlen(message), 0);
}

int receive_line(int sock, char *buffer, int size) {
    memset(buffer, 0, size);
    int total_bytes = 0;
    char *ptr = buffer;
    
    // Read until we get a newline or buffer is full
    while (total_bytes < size - 1) {
        int bytes = recv(sock, ptr, 1, 0);
        if (bytes <= 0) {
            return total_bytes > 0 ? total_bytes : bytes;
        }
        
        total_bytes++;
        if (*ptr == '\n') {
            *ptr = '\0'; // Replace newline with null terminator
            return total_bytes;
        }
        ptr++;
    }
    
    buffer[size - 1] = '\0';
    return total_bytes;
}

int receive_multiline(int sock, char *buffer, int size) {
    memset(buffer, 0, size);
    int bytes = recv(sock, buffer, size - 1, 0);
    return bytes;
}

void print_menu() {
    printf("\n=== MENU ===\n");
    printf("1. Show my sessions\n");
    printf("2. Show online users\n");
    printf("3. Logout\n");
    printf("Choice: ");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_Address> <Port>\n", argv[0]);
        fprintf(stderr, "Example: %s 127.0.0.1 5500\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    if (server_port <= 0 || server_port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char input[256];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        fprintf(stderr, "Invalid IP address: %s\n", server_ip);
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        fprintf(stderr, "Failed to connect to server %s:%d\n", server_ip, server_port);
        fprintf(stderr, "Make sure the server is running!\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", server_ip, server_port);
    fflush(stdout);

    // Receive welcome message
    if (receive_line(sock, buffer, BUFFER_SIZE) > 0) {
        printf("%s", buffer);
        if (buffer[strlen(buffer)-1] != '\n') printf("\n");
        fflush(stdout);
    }

    int logged_in = 0;

    while (1) {
        // Receive server prompt
        int bytes = receive_line(sock, buffer, BUFFER_SIZE);
        if (bytes <= 0) {
            printf("Server closed connection\n");
            break;
        }

        if (strcmp(buffer, "LOGIN") == 0) {
            // Login process
            printf("\n=== LOGIN ===\n");
            fflush(stdout);
            printf("Username: ");
            fflush(stdout);
            if (fgets(input, sizeof(input), stdin) == NULL) break;
            input[strcspn(input, "\n")] = 0;
            strcat(input, "\n");
            send_message(sock, input);

            printf("Password: ");
            fflush(stdout);
            if (fgets(input, sizeof(input), stdin) == NULL) break;
            input[strcspn(input, "\n")] = 0;
            strcat(input, "\n");
            send_message(sock, input);

            // Receive login response
            if (receive_line(sock, buffer, BUFFER_SIZE) <= 0) break;

            if (strcmp(buffer, "OK") == 0) {
                printf("\n✓ Login successful!\n");
                logged_in = 1;
            } else if (strcmp(buffer, "LOCKED") == 0) {
                printf("\n✗ Account is locked due to multiple failed login attempts.\n");
                printf("Please contact administrator.\n");
                break;
            } else if (strcmp(buffer, "WRONG_PASSWORD") == 0) {
                printf("\n✗ Wrong password. Please try again.\n");
            } else if (strcmp(buffer, "USER_NOT_FOUND") == 0) {
                printf("\n✗ User not found. Please check your username.\n");
            }

        } else if (strcmp(buffer, "MENU") == 0 && logged_in) {
            // Show menu and get choice
            print_menu();
            if (fgets(input, sizeof(input), stdin) == NULL) break;
            input[strcspn(input, "\n")] = 0;
            strcat(input, "\n");
            send_message(sock, input);

            // Receive response line by line until empty line (end marker)
            if (receive_line(sock, buffer, BUFFER_SIZE) <= 0) break;

            if (strcmp(buffer, "MY_SESSIONS") == 0) {
                printf("\n=== MY ACTIVE SESSIONS ===\n");
                // Read and display all lines until empty line (end marker)
                while (1) {
                    if (receive_line(sock, buffer, BUFFER_SIZE) <= 0) break;
                    // Empty line marks end of response
                    if (buffer[0] == '\0') break;
                    printf("%s\n", buffer);
                }
                // Continue loop to receive next MENU from server
                continue;

            } else if (strcmp(buffer, "ONLINE_USERS") == 0) {
                printf("\n=== ONLINE USERS ===\n");
                // Read and display all lines until empty line (end marker)
                while (1) {
                    if (receive_line(sock, buffer, BUFFER_SIZE) <= 0) break;
                    // Empty line marks end of response
                    if (buffer[0] == '\0') break;
                    printf("%s\n", buffer);
                }
                // Continue loop to receive next MENU from server
                continue;

            } else if (strcmp(buffer, "LOGOUT") == 0) {
                printf("\n✓ Logged out successfully. Goodbye!\n");
                break;
            }
        }
    }

    close(sock);
    return 0;
}
