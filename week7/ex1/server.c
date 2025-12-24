#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096
#define SHARED_FOLDER "./shared_files"
#define TEMP_FOLDER "./temp_files"

typedef struct {
    int socket;
    int in_use;
    FILE *temp_file;
    char temp_filename[256];
    char output_filename[256];
    unsigned char opcode;
    int key;
} ClientInfo;

// Caesar cipher functions
char encode_char(char c, int key) {
    if (c >= 'a' && c <= 'z') {
        return 'a' + (c - 'a' + key) % 26;
    } else if (c >= 'A' && c <= 'Z') {
        return 'A' + (c - 'A' + key) % 26;
    }
    return c;
}

char decode_char(char c, int key) {
    if (c >= 'a' && c <= 'z') {
        return 'a' + (c - 'a' - key + 26) % 26;
    } else if (c >= 'A' && c <= 'Z') {
        return 'A' + (c - 'A' - key + 26) % 26;
    }
    return c;
}

// Process file using Caesar cipher
int process_file(const char *input_file, const char *output_file, int key, int is_encode) {
    printf("process_file: input=%s, output=%s, key=%d, is_encode=%d\n", 
           input_file, output_file, key, is_encode);
    
    FILE *in = fopen(input_file, "rb");
    if (!in) {
        printf("process_file: ERROR opening input file %s\n", input_file);
        return -1;
    }
    
    FILE *out = fopen(output_file, "wb");
    if (!out) {
        printf("process_file: ERROR opening output file %s\n", output_file);
        fclose(in);
        return -1;
    }
    
    int c;
    int bytes_processed = 0;
    while ((c = fgetc(in)) != EOF) {
        char processed;
        if (is_encode) {
            processed = encode_char((char)c, key);
        } else {
            processed = decode_char((char)c, key);
        }
        fputc(processed, out);
        bytes_processed++;
    }
    
    printf("process_file: Processed %d bytes\n", bytes_processed);
    
    fclose(in);
    fclose(out);
    
    // Verify output file exists and has content
    struct stat st;
    if (stat(output_file, &st) == 0) {
        printf("process_file: Output file size: %ld bytes\n", st.st_size);
    } else {
        printf("process_file: ERROR stat output file\n");
    }
    
    return 0;
}

// Create unique filename
void create_unique_filename(char *filename, const char *folder) {
    static int counter = 0;
    struct stat st;
    
    do {
        snprintf(filename, 256, "%s/file_%d.tmp", folder, counter++);
    } while (stat(filename, &st) == 0);
}

// Send response message
int send_response(int sock, unsigned char opcode, unsigned short length, const unsigned char *payload) {
    unsigned char buffer[BUFFER_SIZE];
    
    buffer[0] = opcode;
    *(unsigned short *)(buffer + 1) = htons(length);
    
    if (length > 0 && payload) {
        memcpy(buffer + 3, payload, length);
    }
    
    int bytes_sent = send(sock, buffer, 3 + length, 0);
    printf("[send_response] opcode=%d, length=%d, bytes_sent=%d\n", opcode, length, bytes_sent);
    return bytes_sent;
}

// Initialize directories
void init_directories() {
    mkdir(SHARED_FOLDER, 0755);
    mkdir(TEMP_FOLDER, 0755);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    int port = atoi(argv[1]);
    init_directories();
    
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(1);
    }
    
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(1);
    }
    
    printf("Server listening on port %d\n", port);
    
    ClientInfo clients[MAX_CLIENTS];
    memset(clients, 0, sizeof(clients));
    
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        
        int max_fd = server_socket;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].in_use) {
                FD_SET(clients[i].socket, &readfds);
                if (clients[i].socket > max_fd) {
                    max_fd = clients[i].socket;
                }
            }
        }
        
        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }
        
        // Handle new connections
        if (FD_ISSET(server_socket, &readfds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
            
            if (client_socket >= 0) {
                int slot = -1;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (!clients[i].in_use) {
                        slot = i;
                        break;
                    }
                }
                
                if (slot >= 0) {
                    clients[slot].socket = client_socket;
                    clients[slot].in_use = 1;
                    clients[slot].temp_file = NULL;
                    
                    // Set TCP_NODELAY for client socket
                    int nodelay = 1;
                    setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
                    
                    printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                } else {
                    close(client_socket);
                }
            }
        }
        
        // Handle client messages
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].in_use) continue;
            
            if (!FD_ISSET(clients[i].socket, &readfds)) continue;
            
            unsigned char buffer[BUFFER_SIZE];
            int n = recv(clients[i].socket, buffer, BUFFER_SIZE, 0);
            
            if (n <= 0) {
                // Client disconnected
                close(clients[i].socket);
                if (clients[i].temp_file) {
                    fclose(clients[i].temp_file);
                }
                if (strlen(clients[i].temp_filename) > 0) {
                    unlink(clients[i].temp_filename);
                }
                if (strlen(clients[i].output_filename) > 0) {
                    unlink(clients[i].output_filename);
                }
                clients[i].in_use = 0;
                printf("Client disconnected\n");
                continue;
            }
            
            unsigned char opcode = buffer[0];
            unsigned short length = ntohs(*(unsigned short *)(buffer + 1));
            unsigned char *payload = buffer + 3;
            
            if (opcode == 0 || opcode == 1) {
                // Encode or Decode request
                if (length != 4) {
                    printf("Client %d: Invalid key length %d (expected 4)\n", i, length);
                    send_response(clients[i].socket, 3, 0, NULL);
                    continue;
                }
                
                clients[i].opcode = opcode;
                clients[i].key = ntohl(*(unsigned int *)payload);
                
                // Create temporary file
                create_unique_filename(clients[i].temp_filename, TEMP_FOLDER);
                clients[i].temp_file = fopen(clients[i].temp_filename, "wb");
                
                if (!clients[i].temp_file) {
                    printf("Client %d: Failed to create temp file %s\n", i, clients[i].temp_filename);
                    send_response(clients[i].socket, 3, 0, NULL);
                    continue;
                }
                
                printf("Client %d: Request %s with key %d\n", i, 
                       opcode == 0 ? "ENCODE" : "DECODE", clients[i].key);
                printf("Client %d: Temp file: %s\n", i, clients[i].temp_filename);
            }
            else if (opcode == 2) {
                // Data transfer
                if (length > 0) {
                    // Receive data
                    if (clients[i].temp_file) {
                        size_t written = fwrite(payload, 1, length, clients[i].temp_file);
                        printf("Client %d: Received %d bytes, wrote %zu bytes\n", i, length, written);
                    } else {
                        printf("Client %d: Temp file not open for writing data\n", i);
                    }
                } else {
                    // End of transfer
                    printf("Client %d: End of file transfer\n", i);
                    if (clients[i].temp_file) {
                        fclose(clients[i].temp_file);
                        clients[i].temp_file = NULL;
                        printf("Client %d: Temp file closed\n", i);
                        
                        // Create output filename
                        static int file_counter = 0;
                        snprintf(clients[i].output_filename, 256, "%s/file_%d.txt", SHARED_FOLDER, file_counter++);
                        
                        // Process file
                        printf("Client %d: Processing file %s -> %s\n", i, clients[i].temp_filename, clients[i].output_filename);
                        int result = process_file(clients[i].temp_filename, 
                                                  clients[i].output_filename,
                                                  clients[i].key,
                                                  clients[i].opcode == 0);
                        
                        printf("Client %d: File processing result: %d\n", i, result);
                        
                        if (result == 0) {
                            // Send processed file
                            printf("Client %d: Opening output file for reading: %s\n", i, clients[i].output_filename);
                            FILE *output_file = fopen(clients[i].output_filename, "rb");
                            if (output_file) {
                                printf("Client %d: Output file opened successfully\n", i);
                                unsigned char file_buffer[BUFFER_SIZE - 3];
                                size_t bytes_read;
                                int chunk_count = 0;
                                
                                while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), output_file)) > 0) {
                                    printf("Client %d: Sending chunk %d with %zu bytes\n", i, chunk_count++, bytes_read);
                                    send_response(clients[i].socket, 2, (unsigned short)bytes_read, file_buffer);
                                }
                                printf("Client %d: Read complete from output file\n", i);
                                fclose(output_file);
                                
                                // Send end of transfer
                                printf("Client %d: Sending end-of-transfer signal\n", i);
                                send_response(clients[i].socket, 2, 0, NULL);
                                
                                // Clean up
                                unlink(clients[i].temp_filename);
                                unlink(clients[i].output_filename);
                                clients[i].temp_filename[0] = 0;
                                clients[i].output_filename[0] = 0;
                                printf("Client %d: Cleanup completed\n", i);
                            } else {
                                printf("Client %d: Failed to open output file %s\n", i, clients[i].output_filename);
                                send_response(clients[i].socket, 3, 0, NULL);
                            }
                        } else {
                            printf("Client %d: File processing failed\n", i);
                            send_response(clients[i].socket, 3, 0, NULL);
                            unlink(clients[i].temp_filename);
                        }
                    } else {
                        printf("Client %d: Temp file not open at end-of-transfer\n", i);
                        send_response(clients[i].socket, 3, 0, NULL);
                    }
                }
            }
            else if (opcode == 3) {
                // Error notification - do nothing
                printf("Client %d: Error notification received\n", i);
            }
        }
    }
    
    close(server_socket);
    return 0;
}
