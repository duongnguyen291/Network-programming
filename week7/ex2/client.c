#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void display_menu() {
    printf("\n=== User Authentication System ===\n");
    printf("1. Login\n");
    printf("2. Logout\n");
    printf("3. Exit\n");
    printf("Select an option (1-3): ");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_Address> <Port>\n", argv[0]);
        exit(1);
    }
    
    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address\n");
        exit(1);
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }
    
    printf("Connected to server at %s:%d\n", server_ip, server_port);
    
    // Receive welcome message
    char buffer[BUFFER_SIZE];
    int n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    
    int logged_in = 0;
    
    while (1) {
        display_menu();
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer
            while (getchar() != '\n');
            printf("Invalid input. Please try again.\n");
            continue;
        }
        // Clear newline
        while (getchar() != '\n');
        
        if (choice == 3) {
            printf("Exiting...\n");
            break;
        }
        
        if (choice == 1) {
            // Login
            char username[50], password[50];
            
            printf("Enter username: ");
            if (fgets(username, sizeof(username), stdin) == NULL) {
                continue;
            }
            username[strcspn(username, "\n")] = '\0';
            
            printf("Enter password: ");
            if (fgets(password, sizeof(password), stdin) == NULL) {
                continue;
            }
            password[strcspn(password, "\n")] = '\0';
            
            // Send login request
            char request[BUFFER_SIZE];
            snprintf(request, sizeof(request), "LOGIN %s %s\n", username, password);
            send(sock, request, strlen(request), 0);
            
            // Receive response
            n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (n > 0) {
                buffer[n] = '\0';
                printf("\nServer response: %s", buffer);
                
                if (strncmp(buffer, "OK:", 3) == 0) {
                    logged_in = 1;
                }
            } else {
                printf("Connection lost\n");
                break;
            }
        }
        else if (choice == 2) {
            // Logout
            char request[] = "LOGOUT\n";
            send(sock, request, strlen(request), 0);
            
            // Receive response
            n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (n > 0) {
                buffer[n] = '\0';
                printf("\nServer response: %s", buffer);
                
                if (strncmp(buffer, "OK:", 3) == 0) {
                    logged_in = 0;
                }
            } else {
                printf("Connection lost\n");
                break;
            }
        }
        else {
            printf("Invalid choice. Please try again.\n");
        }
    }
    
    close(sock);
    printf("Connection closed.\n");
    return 0;
}
