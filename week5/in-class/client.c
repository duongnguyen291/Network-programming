#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define BUFF_SIZE 1024
#define MAX_USERNAME 50
#define MAX_MESSAGE 1000

// Message types
#define MSG_LOGIN 0x01
#define MSG_MESSAGE 0x02
#define MSG_ACK 0x03
#define MSG_ERROR 0x04
#define MSG_LOGOUT 0x05

// Message structure
typedef struct {
    unsigned char type;
    unsigned int length;
    char* payload;
} Message;

// Function prototypes
void send_message(int connfd, unsigned char type, const char* payload);
Message* receive_message(int connfd);
void free_message(Message* msg);
int login_to_server(int connfd);
void send_message_to_server(int connfd);
void logout_from_server(int connfd);
void show_menu();

// Send message to server
void send_message(int connfd, unsigned char type, const char* payload) {
    unsigned int length = strlen(payload);
    unsigned int net_length = htonl(length);
    
    // Send message type
    send(connfd, &type, 1, 0);
    
    // Send payload length
    send(connfd, &net_length, 4, 0);
    
    // Send payload
    send(connfd, payload, length, 0);
    
    printf("Sent: Type=%d, Length=%d, Payload='%s'\n", type, length, payload);
}

// Receive message from server
Message* receive_message(int connfd) {
    Message* msg = malloc(sizeof(Message));
    if (!msg) return NULL;
    
    // Receive message type
    if (recv(connfd, &msg->type, 1, 0) <= 0) {
        free(msg);
        return NULL;
    }
    
    // Receive payload length
    unsigned int net_length;
    if (recv(connfd, &net_length, 4, 0) <= 0) {
        free(msg);
        return NULL;
    }
    msg->length = ntohl(net_length);
    
    // Validate length
    if (msg->length > BUFF_SIZE) {
        free(msg);
        return NULL;
    }
    
    // Receive payload
    msg->payload = malloc(msg->length + 1);
    if (!msg->payload) {
        free(msg);
        return NULL;
    }
    
    if (recv(connfd, msg->payload, msg->length, 0) <= 0) {
        free(msg->payload);
        free(msg);
        return NULL;
    }
    
    msg->payload[msg->length] = '\0';
    printf("Received: Type=%d, Length=%d, Payload='%s'\n", msg->type, msg->length, msg->payload);
    
    return msg;
}

// Free message memory
void free_message(Message* msg) {
    if (msg) {
        if (msg->payload) {
            free(msg->payload);
        }
        free(msg);
    }
}

// Login to server
int login_to_server(int connfd) {
    char username[MAX_USERNAME + 1];
    Message* response;
    
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    
    // Remove newline
    char* newline = strchr(username, '\n');
    if (newline) *newline = '\0';
    
    if (strlen(username) == 0) {
        printf("Username cannot be empty\n");
        return 0;
    }
    
    // Send login message
    send_message(connfd, MSG_LOGIN, username);
    
    // Receive response
    response = receive_message(connfd);
    if (!response) {
        printf("Server disconnected\n");
        return 0;
    }
    
    if (response->type == MSG_ACK) {
        printf("Login successful: %s\n", response->payload);
        free_message(response);
        return 1;
    } else if (response->type == MSG_ERROR) {
        printf("Login failed: %s\n", response->payload);
        free_message(response);
        return 0;
    } else {
        printf("Unexpected response from server\n");
        free_message(response);
        return 0;
    }
}

// Send message to server
void send_message_to_server(int connfd) {
    char message[MAX_MESSAGE + 1];
    Message* response;
    
    printf("Enter message: ");
    fgets(message, sizeof(message), stdin);
    
    // Remove newline
    char* newline = strchr(message, '\n');
    if (newline) *newline = '\0';
    
    if (strlen(message) == 0) {
        printf("Message cannot be empty\n");
        return;
    }
    
    // Send message
    send_message(connfd, MSG_MESSAGE, message);
    
    // Receive response
    response = receive_message(connfd);
    if (!response) {
        printf("Server disconnected\n");
        return;
    }
    
    if (response->type == MSG_ACK) {
        printf("Message sent successfully: %s\n", response->payload);
    } else if (response->type == MSG_ERROR) {
        printf("Message failed: %s\n", response->payload);
    } else {
        printf("Unexpected response from server\n");
    }
    
    free_message(response);
}

// Logout from server
void logout_from_server(int connfd) {
    Message* response;
    
    // Send logout message
    send_message(connfd, MSG_LOGOUT, "");
    
    // Receive response
    response = receive_message(connfd);
    if (!response) {
        printf("Server disconnected\n");
        return;
    }
    
    if (response->type == MSG_ACK) {
        printf("Logout successful: %s\n", response->payload);
    } else if (response->type == MSG_ERROR) {
        printf("Logout failed: %s\n", response->payload);
    } else {
        printf("Unexpected response from server\n");
    }
    
    free_message(response);
}

// Show menu
void show_menu() {
    printf("\n=== TCP Chat Client Menu ===\n");
    printf("1. Login\n");
    printf("2. Send Message\n");
    printf("3. Logout\n");
    printf("4. Exit\n");
    printf("Enter choice: ");
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port_number>\n", argv[0]);
        printf("Example: %s 127.0.0.1 5500\n", argv[0]);
        exit(1);
    }
    
    char* server_ip = argv[1];
    int port = atoi(argv[2]);
    
    if (port <= 0 || port > 65535) {
        printf("Error: Invalid port number. Port must be between 1 and 65535\n");
        exit(1);
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }
#endif

    int sockfd;
    struct sockaddr_in servaddr;
    int choice;
    int logged_in = 0;

    // 1. Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket() failed");
        exit(1);
    }

    // 2. Set server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
#ifdef _WIN32
    servaddr.sin_addr.s_addr = inet_addr(server_ip);
#else
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);
#endif

    // 3. Connect to server
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect() failed");
        exit(1);
    }
    
    printf("Connected to server %s:%d\n", server_ip, port);
    printf("TCP Chat Client\n");

    // 4. Main menu loop
    while (1) {
        show_menu();
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        while (getchar() != '\n'); // Clear input buffer
        
        switch (choice) {
            case 1: // Login
                if (logged_in) {
                    printf("Already logged in\n");
                } else {
                    if (login_to_server(sockfd)) {
                        logged_in = 1;
                    }
                }
                break;
                
            case 2: // Send Message
                if (!logged_in) {
                    printf("Please login first\n");
                } else {
                    send_message_to_server(sockfd);
                }
                break;
                
            case 3: // Logout
                if (!logged_in) {
                    printf("Not logged in\n");
                } else {
                    logout_from_server(sockfd);
                    logged_in = 0;
                }
                break;
                
            case 4: // Exit
                if (logged_in) {
                    logout_from_server(sockfd);
                }
                printf("Disconnecting from server...\n");
                goto cleanup;
                
            default:
                printf("Invalid choice\n");
                break;
        }
    }
    
cleanup:
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
    return 0;
}
