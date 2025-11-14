#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
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
#define LOG_DIR "logs"

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

// Client session structure
typedef struct {
    int connfd;
    char username[MAX_USERNAME + 1];
    FILE* logfile;
    int logged_in;
} ClientSession;

// Function prototypes
void create_log_directory();
int validate_username(const char* username);
void send_message(int connfd, unsigned char type, const char* payload);
Message* receive_message(int connfd);
void free_message(Message* msg);
void log_message(FILE* logfile, const char* username, const char* message);
void handle_client(ClientSession* session);
void cleanup_session(ClientSession* session);

// Create log directory
void create_log_directory() {
#ifdef _WIN32
    if (mkdir(LOG_DIR) != 0) {
        // Directory already exists or error
        printf("Log directory: %s/\n", LOG_DIR);
    } else {
        printf("Created log directory: %s/\n", LOG_DIR);
    }
#else
    if (mkdir(LOG_DIR, 0755) != 0) {
        printf("Log directory: %s/\n", LOG_DIR);
    } else {
        printf("Created log directory: %s/\n", LOG_DIR);
    }
#endif
}

// Validate username format
int validate_username(const char* username) {
    if (strlen(username) == 0 || strlen(username) > MAX_USERNAME) {
        return 0;
    }
    
    for (int i = 0; username[i] != '\0'; i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            return 0;
        }
    }
    
    return 1;
}

// Send message to client
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

// Receive message from client
Message* receive_message(int connfd) {
    Message* msg = malloc(sizeof(Message));
    if (!msg) return NULL;
    
    // Receive message type
    int bytes_received = recv(connfd, &msg->type, 1, 0);
    printf("DEBUG: Received %d bytes for message type\n", bytes_received);
    if (bytes_received <= 0) {
        printf("DEBUG: Failed to receive message type\n");
        free(msg);
        return NULL;
    }
    
    // Receive payload length
    unsigned int net_length;
    int length_bytes = recv(connfd, &net_length, 4, 0);
    printf("DEBUG: Received %d bytes for message length\n", length_bytes);
    if (length_bytes <= 0) {
        printf("DEBUG: Failed to receive message length\n");
        free(msg);
        return NULL;
    }
    msg->length = ntohl(net_length);
    printf("DEBUG: Message length = %d\n", msg->length);
    
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
    
    if (msg->length > 0) {
        int payload_bytes = recv(connfd, msg->payload, msg->length, 0);
        printf("DEBUG: Received %d bytes for payload\n", payload_bytes);
        if (payload_bytes <= 0) {
            printf("DEBUG: Failed to receive payload\n");
            free(msg->payload);
            free(msg);
            return NULL;
        }
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

// Log message to file
void log_message(FILE* logfile, const char* username, const char* message) {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    
    fprintf(logfile, "[%04d-%02d-%02d %02d:%02d:%02d] %s: %s\n",
            timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
            username, message);
    fflush(logfile);
}

// Handle client session
void handle_client(ClientSession* session) {
    Message* msg;
    
    printf("Client session started\n");
    
    while (1) {
        msg = receive_message(session->connfd);
        if (!msg) {
            printf("Client disconnected or error occurred\n");
            break;
        }
        
        printf("Processing message: Type=%d, Length=%d, Payload='%s'\n", 
               msg->type, msg->length, msg->payload);
        
        switch (msg->type) {
            case MSG_LOGIN:
                if (session->logged_in) {
                    send_message(session->connfd, MSG_ERROR, "Already logged in");
                } else if (!validate_username(msg->payload)) {
                    send_message(session->connfd, MSG_ERROR, "Invalid username format");
                } else {
                    strcpy(session->username, msg->payload);
                    
                    // Create log file
                    char logfile_path[256];
                    snprintf(logfile_path, sizeof(logfile_path), "%s/%s.log", LOG_DIR, session->username);
                    session->logfile = fopen(logfile_path, "a");
                    
                    if (!session->logfile) {
                        send_message(session->connfd, MSG_ERROR, "Cannot create log file");
                    } else {
                        session->logged_in = 1;
                        send_message(session->connfd, MSG_ACK, "Login successful");
                        printf("User '%s' logged in\n", session->username);
                    }
                }
                break;
                
            case MSG_MESSAGE:
                if (!session->logged_in) {
                    send_message(session->connfd, MSG_ERROR, "Not logged in");
                } else if (strlen(msg->payload) > MAX_MESSAGE) {
                    send_message(session->connfd, MSG_ERROR, "Message too long");
                } else {
                    log_message(session->logfile, session->username, msg->payload);
                    send_message(session->connfd, MSG_ACK, "Message logged");
                    printf("Message from '%s': %s\n", session->username, msg->payload);
                }
                break;
                
            case MSG_LOGOUT:
                if (session->logged_in) {
                    send_message(session->connfd, MSG_ACK, "Logout successful");
                    printf("User '%s' logged out\n", session->username);
                    cleanup_session(session);
                    printf("Logout response sent, continuing session...\n");
                } else {
                    send_message(session->connfd, MSG_ERROR, "Not logged in");
                    printf("Logout error sent, continuing session...\n");
                }
                // Don't return immediately, let client receive response
                break;
                
            default:
                send_message(session->connfd, MSG_ERROR, "Unknown message type");
                break;
        }
        
        free_message(msg);
    }
    
    cleanup_session(session);
}

// Cleanup client session
void cleanup_session(ClientSession* session) {
    if (session->logfile) {
        fclose(session->logfile);
        session->logfile = NULL;
    }
    session->logged_in = 0;
    printf("Client session ended\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port_number>\n", argv[0]);
        printf("Example: %s 5500\n", argv[0]);
        exit(1);
    }
    
    int port = atoi(argv[1]);
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

    int listenfd, connfd;
    struct sockaddr_in servaddr, clientaddr;
    socklen_t len = sizeof(clientaddr);
    ClientSession session;

    // Create log directory
    create_log_directory();

    // 1. Create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket() failed");
        exit(1);
    }

    // 2. Set server address
#ifdef _WIN32
    memset(&servaddr, 0, sizeof(servaddr));
#else
    bzero(&servaddr, sizeof(servaddr));
#endif
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    // 3. Listen for connections
    listen(listenfd, 5);
    printf("TCP Chat Server started on port %d\n", port);
    printf("Log directory: %s/\n", LOG_DIR);
    printf("Waiting for connections...\n");

    // 4. Accept connections
    while (1) {
        printf("\nWaiting for client connection...\n");
        connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
        if (connfd < 0) {
            perror("accept() failed");
            continue;
        }
        
        printf("Client connected: %s:%d\n", 
               inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // Initialize session
        session.connfd = connfd;
        session.logfile = NULL;
        session.logged_in = 0;
        memset(session.username, 0, sizeof(session.username));

        // Handle client
        handle_client(&session);
        
        // Close connection
#ifdef _WIN32
        closesocket(connfd);
#else
        close(connfd);
#endif
    }

#ifdef _WIN32
    closesocket(listenfd);
    WSACleanup();
#else
    close(listenfd);
#endif
    return 0;
}
