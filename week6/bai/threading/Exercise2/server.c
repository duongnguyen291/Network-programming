#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define MAX_ACCOUNTS 100
#define MAX_SESSIONS 1000
#define ACCOUNT_FILE "account.txt"

// Account structure
typedef struct {
    char username[50];
    char password[50];
    int status; // 0 = locked, 1 = unlocked
    int failed_attempts;
} Account;

// Session structure
typedef struct {
    char username[50];
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    time_t connect_time;
    int active;
} Session;

// Global variables
Account accounts[MAX_ACCOUNTS];
int account_count = 0;
Session sessions[MAX_SESSIONS];
int session_count = 0;
pthread_mutex_t accounts_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sessions_mutex = PTHREAD_MUTEX_INITIALIZER;

// Client thread data
typedef struct {
    int client_sock;
    struct sockaddr_in client_addr;
    int session_id;
} client_data_t;

// Function prototypes
void load_accounts();
void save_accounts();
int authenticate(const char *username, const char *password);
void add_session(const char *username, const char *ip, int port);
void remove_session(int session_id);
void *handle_client(void *arg);
void send_message(int sock, const char *message);
int receive_message(int sock, char *buffer, int size);

// Load accounts from file
void load_accounts() {
    FILE *fp = fopen(ACCOUNT_FILE, "r");
    if (fp == NULL) {
        printf("Warning: %s not found. Creating empty account list.\n", ACCOUNT_FILE);
        return;
    }

    account_count = 0;
    char line[200];
    
    // Skip header line
    if (fgets(line, sizeof(line), fp) != NULL) {
        while (fgets(line, sizeof(line), fp) != NULL && account_count < MAX_ACCOUNTS) {
            // Remove newline
            line[strcspn(line, "\n")] = 0;
            
            if (sscanf(line, "%s %s %d", 
                       accounts[account_count].username,
                       accounts[account_count].password,
                       &accounts[account_count].status) == 3) {
                accounts[account_count].failed_attempts = 0;
                account_count++;
            }
        }
    }
    
    fclose(fp);
    printf("Loaded %d accounts from %s\n", account_count, ACCOUNT_FILE);
}

// Save accounts to file
void save_accounts() {
    FILE *fp = fopen(ACCOUNT_FILE, "w");
    if (fp == NULL) {
        perror("Error saving accounts");
        return;
    }

    fprintf(fp, "Username Password Status\n");
    for (int i = 0; i < account_count; i++) {
        fprintf(fp, "%s %s %d\n", 
                accounts[i].username,
                accounts[i].password,
                accounts[i].status);
    }
    
    fclose(fp);
}

// Authenticate user
int authenticate(const char *username, const char *password) {
    pthread_mutex_lock(&accounts_mutex);
    
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            // Check if account is locked
            if (accounts[i].status == 0) {
                pthread_mutex_unlock(&accounts_mutex);
                return -2; // Account locked
            }
            
            // Check password
            if (strcmp(accounts[i].password, password) == 0) {
                accounts[i].failed_attempts = 0;
                pthread_mutex_unlock(&accounts_mutex);
                return 1; // Success
            } else {
                accounts[i].failed_attempts++;
                if (accounts[i].failed_attempts >= 5) {
                    accounts[i].status = 0; // Lock account
                    save_accounts();
                    pthread_mutex_unlock(&accounts_mutex);
                    return -3; // Account locked due to failed attempts
                }
                pthread_mutex_unlock(&accounts_mutex);
                return 0; // Wrong password
            }
        }
    }
    
    pthread_mutex_unlock(&accounts_mutex);
    return -1; // User not found
}

// Add session
void add_session(const char *username, const char *ip, int port) {
    pthread_mutex_lock(&sessions_mutex);
    
    if (session_count < MAX_SESSIONS) {
        strcpy(sessions[session_count].username, username);
        strcpy(sessions[session_count].client_ip, ip);
        sessions[session_count].client_port = port;
        sessions[session_count].connect_time = time(NULL);
        sessions[session_count].active = 1;
        session_count++;
    }
    
    pthread_mutex_unlock(&sessions_mutex);
}

// Remove session
void remove_session(int session_id) {
    pthread_mutex_lock(&sessions_mutex);
    
    if (session_id >= 0 && session_id < session_count) {
        sessions[session_id].active = 0;
    }
    
    pthread_mutex_unlock(&sessions_mutex);
}

// Send message to client
void send_message(int sock, const char *message) {
    send(sock, message, strlen(message), 0);
}

// Receive message from client
int receive_message(int sock, char *buffer, int size) {
    memset(buffer, 0, size);
    int bytes = recv(sock, buffer, size - 1, 0);
    if (bytes > 0) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
    }
    return bytes;
}

// Handle client thread
void *handle_client(void *arg) {
    client_data_t *data = (client_data_t *)arg;
    int client_sock = data->client_sock;
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    
    inet_ntop(AF_INET, &(data->client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    client_port = ntohs(data->client_addr.sin_port);
    
    printf("Client connected from %s:%d\n", client_ip, client_port);
    
    char buffer[BUFFER_SIZE];
    char username[50] = "";
    int logged_in = 0;
    int session_id = -1;
    
    send_message(client_sock, "Welcome to Authentication Server\n");
    
    while (1) {
        if (!logged_in) {
            send_message(client_sock, "LOGIN\n");
            
            // Receive username
            if (receive_message(client_sock, buffer, BUFFER_SIZE) <= 0) break;
            strcpy(username, buffer);
            
            // Receive password
            if (receive_message(client_sock, buffer, BUFFER_SIZE) <= 0) break;
            
            int auth_result = authenticate(username, buffer);
            
            if (auth_result == 1) {
                send_message(client_sock, "OK\n");
                logged_in = 1;
                session_id = session_count;
                add_session(username, client_ip, client_port);
                printf("User '%s' logged in from %s:%d\n", username, client_ip, client_port);
            } else if (auth_result == -2 || auth_result == -3) {
                send_message(client_sock, "LOCKED\n");
                printf("Login attempt for locked account '%s' from %s:%d\n", username, client_ip, client_port);
            } else if (auth_result == 0) {
                send_message(client_sock, "WRONG_PASSWORD\n");
                printf("Wrong password for '%s' from %s:%d\n", username, client_ip, client_port);
            } else {
                send_message(client_sock, "USER_NOT_FOUND\n");
                printf("User not found: '%s' from %s:%d\n", username, client_ip, client_port);
            }
        } else {
            send_message(client_sock, "MENU\n");
            
            if (receive_message(client_sock, buffer, BUFFER_SIZE) <= 0) break;
            
            if (strcmp(buffer, "1") == 0) {
                // Show my sessions
                pthread_mutex_lock(&sessions_mutex);
                char response[4096] = "MY_SESSIONS\n";
                int count = 0;
                
                for (int i = 0; i < session_count; i++) {
                    if (sessions[i].active && strcmp(sessions[i].username, username) == 0) {
                        char line[256];
                        struct tm *tm_info = localtime(&sessions[i].connect_time);
                        char time_str[64];
                        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
                        
                        snprintf(line, sizeof(line), "%d. IP: %s:%d, Connected: %s\n",
                                ++count, sessions[i].client_ip, sessions[i].client_port, time_str);
                        strcat(response, line);
                    }
                }
                
                if (count == 0) {
                    strcat(response, "No active sessions found.\n");
                }
                
                pthread_mutex_unlock(&sessions_mutex);
                send_message(client_sock, response);
                send_message(client_sock, "\n"); // Empty line to mark end of response
                // Response sent, will continue loop to send MENU again
                
            } else if (strcmp(buffer, "2") == 0) {
                // Show online users
                pthread_mutex_lock(&sessions_mutex);
                char response[4096] = "ONLINE_USERS\n";
                char online_users[MAX_ACCOUNTS][50];
                int online_count = 0;
                
                // Get unique online users
                for (int i = 0; i < session_count; i++) {
                    if (sessions[i].active) {
                        int found = 0;
                        for (int j = 0; j < online_count; j++) {
                            if (strcmp(online_users[j], sessions[i].username) == 0) {
                                found = 1;
                                break;
                            }
                        }
                        if (!found && online_count < MAX_ACCOUNTS) {
                            strcpy(online_users[online_count++], sessions[i].username);
                        }
                    }
                }
                
                for (int i = 0; i < online_count; i++) {
                    char line[100];
                    snprintf(line, sizeof(line), "%d. %s\n", i + 1, online_users[i]);
                    strcat(response, line);
                }
                
                if (online_count == 0) {
                    strcat(response, "No users online.\n");
                }
                
                pthread_mutex_unlock(&sessions_mutex);
                send_message(client_sock, response);
                send_message(client_sock, "\n"); // Empty line to mark end of response
                // Response sent, will continue loop to send MENU again
                
            } else if (strcmp(buffer, "3") == 0) {
                // Logout
                send_message(client_sock, "LOGOUT\n");
                printf("User '%s' logged out from %s:%d\n", username, client_ip, client_port);
                break;
            }
        }
    }
    
    if (session_id >= 0) {
        remove_session(session_id);
    }
    
    printf("Client disconnected: %s:%d\n", client_ip, client_port);
    close(client_sock);
    free(data);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    // Load accounts
    load_accounts();

    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;

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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 10) < 0) {
        perror("listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Authentication Server started on port %d\n", port);
    printf("Using threading technique for multiple clients\n");

    while (1) {
        client_addr_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        client_data_t *data = malloc(sizeof(client_data_t));
        if (data == NULL) {
            perror("malloc");
            close(client_sock);
            continue;
        }

        data->client_sock = client_sock;
        data->client_addr = client_addr;
        data->session_id = -1;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)data) != 0) {
            perror("pthread_create");
            free(data);
            close(client_sock);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_sock);
    return 0;
}
