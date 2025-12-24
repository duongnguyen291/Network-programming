#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define ACCOUNT_FILE "account.txt"
#define MAX_ACCOUNTS 100
#define MAX_FAILED_ATTEMPTS 3

typedef struct {
    char username[50];
    char password[50];
    int status; // 0: locked, 1: unlocked
    int failed_attempts;
} Account;

typedef struct {
    int fd;
    char username[50];
    int logged_in;
} Client;

Account accounts[MAX_ACCOUNTS];
int account_count = 0;
Client clients[MAX_CLIENTS];

// Load accounts from file
int load_accounts() {
    FILE *file = fopen(ACCOUNT_FILE, "r");
    if (!file) {
        perror("Error opening account file");
        return -1;
    }
    
    account_count = 0;
    char line[256];
    
    while (fgets(line, sizeof(line), file) && account_count < MAX_ACCOUNTS) {
        // Skip empty lines and comments
        if (line[0] == '\n' || line[0] == '#') continue;
        
        if (sscanf(line, "%s %s %d", 
                   accounts[account_count].username,
                   accounts[account_count].password,
                   &accounts[account_count].status) == 3) {
            accounts[account_count].failed_attempts = 0;
            account_count++;
        }
    }
    
    fclose(file);
    printf("Loaded %d accounts\n", account_count);
    return account_count;
}

// Save accounts to file
void save_accounts() {
    FILE *file = fopen(ACCOUNT_FILE, "w");
    if (!file) {
        perror("Error saving accounts");
        return;
    }
    
    for (int i = 0; i < account_count; i++) {
        fprintf(file, "%s %s %d\n", 
                accounts[i].username,
                accounts[i].password,
                accounts[i].status);
    }
    
    fclose(file);
}

// Find account by username
Account* find_account(const char *username) {
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            return &accounts[i];
        }
    }
    return NULL;
}

// Check if account is already logged in on this client
int is_logged_in_on_client(int client_idx) {
    return clients[client_idx].logged_in;
}

// Check if username is logged in on this specific client
int is_username_logged_in_on_client(int client_idx, const char *username) {
    if (clients[client_idx].logged_in && 
        strcmp(clients[client_idx].username, username) == 0) {
        return 1;
    }
    return 0;
}

// Handle login request
void handle_login(int client_idx, const char *username, const char *password) {
    char response[BUFFER_SIZE];
    
    // Check if client already logged in
    if (is_logged_in_on_client(client_idx)) {
        sprintf(response, "ERROR: Already logged in as %s\n", clients[client_idx].username);
        send(clients[client_idx].fd, response, strlen(response), 0);
        return;
    }
    
    // Find account
    Account *account = find_account(username);
    if (!account) {
        sprintf(response, "ERROR: Account does not exist\n");
        send(clients[client_idx].fd, response, strlen(response), 0);
        return;
    }
    
    // Check if account is locked
    if (account->status == 0) {
        sprintf(response, "ERROR: Account is locked\n");
        send(clients[client_idx].fd, response, strlen(response), 0);
        return;
    }
    
    // Check password
    if (strcmp(account->password, password) != 0) {
        account->failed_attempts++;
        
        if (account->failed_attempts >= MAX_FAILED_ATTEMPTS) {
            account->status = 0; // Lock account
            save_accounts();
            sprintf(response, "ERROR: Wrong password. Account is now locked\n");
        } else {
            sprintf(response, "ERROR: Wrong password. %d attempts remaining\n", 
                    MAX_FAILED_ATTEMPTS - account->failed_attempts);
        }
        send(clients[client_idx].fd, response, strlen(response), 0);
        return;
    }
    
    // Successful login
    account->failed_attempts = 0;
    clients[client_idx].logged_in = 1;
    strcpy(clients[client_idx].username, username);
    
    sprintf(response, "OK: Login successful. Welcome %s!\n", username);
    send(clients[client_idx].fd, response, strlen(response), 0);
    
    printf("User %s logged in from client %d\n", username, client_idx);
}

// Handle logout request
void handle_logout(int client_idx) {
    char response[BUFFER_SIZE];
    
    if (!is_logged_in_on_client(client_idx)) {
        sprintf(response, "ERROR: Not logged in\n");
        send(clients[client_idx].fd, response, strlen(response), 0);
        return;
    }
    
    printf("User %s logged out from client %d\n", clients[client_idx].username, client_idx);
    
    sprintf(response, "OK: Logout successful. Goodbye %s!\n", clients[client_idx].username);
    send(clients[client_idx].fd, response, strlen(response), 0);
    
    clients[client_idx].logged_in = 0;
    clients[client_idx].username[0] = '\0';
}

// Handle client message
void handle_message(int client_idx, const char *message) {
    char cmd[20], username[50], password[50];
    
    // Parse command
    if (sscanf(message, "%s", cmd) != 1) {
        return;
    }
    
    if (strcmp(cmd, "LOGIN") == 0) {
        if (sscanf(message, "%s %s %s", cmd, username, password) == 3) {
            handle_login(client_idx, username, password);
        } else {
            char response[] = "ERROR: Invalid LOGIN format. Use: LOGIN username password\n";
            send(clients[client_idx].fd, response, strlen(response), 0);
        }
    }
    else if (strcmp(cmd, "LOGOUT") == 0) {
        handle_logout(client_idx);
    }
    else {
        char response[] = "ERROR: Unknown command\n";
        send(clients[client_idx].fd, response, strlen(response), 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    int port = atoi(argv[1]);
    
    // Load accounts
    if (load_accounts() < 0) {
        fprintf(stderr, "Failed to load accounts\n");
        exit(1);
    }
    
    // Create server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(1);
    }
    
    printf("Server listening on port %d\n", port);
    
    // Initialize poll array
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;
    
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    
    // Initialize clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].logged_in = 0;
        clients[i].username[0] = '\0';
    }
    
    while (1) {
        int ret = poll(fds, nfds, -1);
        
        if (ret < 0) {
            perror("poll");
            break;
        }
        
        // Check for new connections
        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            
            if (client_fd >= 0) {
                // Find empty slot
                int slot = -1;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd == -1) {
                        slot = i;
                        break;
                    }
                }
                
                if (slot >= 0) {
                    clients[slot].fd = client_fd;
                    clients[slot].logged_in = 0;
                    clients[slot].username[0] = '\0';
                    
                    fds[nfds].fd = client_fd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                    
                    printf("New client connected from %s:%d (slot %d)\n", 
                           inet_ntoa(client_addr.sin_addr), 
                           ntohs(client_addr.sin_port),
                           slot);
                    
                    char welcome[] = "Welcome! Please login.\n";
                    send(client_fd, welcome, strlen(welcome), 0);
                } else {
                    close(client_fd);
                    printf("Max clients reached, connection rejected\n");
                }
            }
        }
        
        // Check existing clients
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                char buffer[BUFFER_SIZE];
                int n = recv(fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
                
                if (n <= 0) {
                    // Client disconnected
                    int client_idx = -1;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].fd == fds[i].fd) {
                            client_idx = j;
                            break;
                        }
                    }
                    
                    if (client_idx >= 0) {
                        if (clients[client_idx].logged_in) {
                            printf("Client %d (%s) disconnected\n", 
                                   client_idx, clients[client_idx].username);
                        } else {
                            printf("Client %d disconnected\n", client_idx);
                        }
                        
                        close(clients[client_idx].fd);
                        clients[client_idx].fd = -1;
                        clients[client_idx].logged_in = 0;
                        clients[client_idx].username[0] = '\0';
                    }
                    
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                } else {
                    buffer[n] = '\0';
                    
                    // Remove newline
                    char *newline = strchr(buffer, '\n');
                    if (newline) *newline = '\0';
                    newline = strchr(buffer, '\r');
                    if (newline) *newline = '\0';
                    
                    // Find client index
                    int client_idx = -1;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].fd == fds[i].fd) {
                            client_idx = j;
                            break;
                        }
                    }
                    
                    if (client_idx >= 0 && strlen(buffer) > 0) {
                        printf("Client %d: %s\n", client_idx, buffer);
                        handle_message(client_idx, buffer);
                    }
                }
            }
        }
    }
    
    close(server_fd);
    return 0;
}
