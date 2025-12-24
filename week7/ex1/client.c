#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

// Send message to server
int send_message(int sock, unsigned char opcode, unsigned short length, const unsigned char *payload) {
    unsigned char buffer[BUFFER_SIZE];
    
    buffer[0] = opcode;
    *(unsigned short *)(buffer + 1) = htons(length);
    
    if (length > 0 && payload) {
        memcpy(buffer + 3, payload, length);
    }
    
    return send(sock, buffer, 3 + length, 0);
}

// Receive message from server
int receive_message(int sock, unsigned char *opcode, unsigned short *length, unsigned char *payload) {
    unsigned char header[3];
    
    // First, receive the 3-byte header
    int total_received = 0;
    while (total_received < 3) {
        int n = recv(sock, header + total_received, 3 - total_received, 0);
        if (n <= 0) {
            return -1;
        }
        total_received += n;
    }
    
    *opcode = header[0];
    *length = ntohs(*(unsigned short *)(header + 1));
    
    printf("[receive_message] Received header: opcode=%d, length=%d\n", *opcode, *length);
    
    // Then, receive the payload if length > 0
    if (*length > 0) {
        total_received = 0;
        while (total_received < *length) {
            int n = recv(sock, payload + total_received, *length - total_received, 0);
            if (n <= 0) {
                return -1;
            }
            total_received += n;
        }
    }
    
    return 3 + *length;
}

void display_menu() {
    printf("\n=== Caesar Cipher File Encoder/Decoder ===\n");
    printf("1. Encode file\n");
    printf("2. Decode file\n");
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
    
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }
    
    int opt = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
        perror("setsockopt TCP_NODELAY");
        exit(1);
    }
    
    printf("Connected to server at %s:%d\n", server_ip, server_port);
    
    while (1) {
        display_menu();
        int choice;
        scanf("%d", &choice);
        
        if (choice == 3) {
            printf("Exiting...\n");
            break;
        }
        
        if (choice != 1 && choice != 2) {
            printf("Invalid choice. Please try again.\n");
            continue;
        }
        
        // Get key from user
        printf("Enter the key (0-25): ");
        int key;
        scanf("%d", &key);
        
        if (key < 0 || key > 25) {
            printf("Invalid key. Please use a value between 0 and 25.\n");
            continue;
        }
        
        // Get filename from user
        printf("Enter the filename: ");
        char filename[256];
        scanf("%255s", filename);
        
        // Check if file exists
        if (access(filename, F_OK) == -1) {
            printf("File not found: %s\n", filename);
            continue;
        }
        
        // Send encode/decode request with key
        unsigned char opcode = (choice == 1) ? 0 : 1;
        unsigned int key_network = htonl((unsigned int)key);
        
        if (send_message(sock, opcode, 4, (unsigned char *)&key_network) < 0) {
            perror("send");
            break;
        }
        
        printf("Sending %s request with key %d for file: %s\n", 
               choice == 1 ? "ENCODE" : "DECODE", key, filename);
        
        // Small delay to ensure request is processed
        usleep(10000); // 10ms delay
        
        // Send file data
        FILE *file = fopen(filename, "rb");
        if (!file) {
            perror("fopen");
            continue;
        }
        
        unsigned char file_buffer[BUFFER_SIZE - 3];
        size_t bytes_read;
        int chunk_count = 0;
        
        while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
            printf("Sending data chunk %d: %zu bytes\n", chunk_count++, bytes_read);
            if (send_message(sock, 2, (unsigned short)bytes_read, file_buffer) < 0) {
                perror("send");
                fclose(file);
                break;
            }
            usleep(5000); // 5ms delay between chunks
        }
        fclose(file);
        
        // Send end of transfer
        printf("Sending end-of-transfer signal\n");
        if (send_message(sock, 2, 0, NULL) < 0) {
            perror("send");
            break;
        }
        
        usleep(10000); // 10ms delay before waiting for response
        
        printf("File sent. Waiting for server response...\n");
        
        // Receive processed file
        unsigned char opcode_resp;
        unsigned short length_resp;
        unsigned char payload[BUFFER_SIZE];
        
        FILE *output_file = fopen("received_file.txt", "wb");
        if (!output_file) {
            perror("fopen output");
            break;
        }
        
        int transfer_complete = 0;
        int chunk_num = 0;
        while (!transfer_complete) {
            printf("Waiting for server response chunk %d...\n", chunk_num);
            if (receive_message(sock, &opcode_resp, &length_resp, payload) < 0) {
                perror("recv");
                printf("ERROR: Failed to receive from server\n");
                break;
            }
            
            printf("Received opcode=%d, length=%d\n", opcode_resp, length_resp);
            
            if (opcode_resp == 3) {
                printf("Error: Server reported an error\n");
                transfer_complete = 1;
            } else if (opcode_resp == 2) {
                if (length_resp > 0) {
                    fwrite(payload, 1, length_resp, output_file);
                    printf("Received and saved %d bytes\n", length_resp);
                    chunk_num++;
                } else {
                    // End of transfer
                    transfer_complete = 1;
                    printf("File received successfully and saved as 'received_file.txt'\n");
                }
            }
        }
        
        fclose(output_file);
    }
    
    close(sock);
    printf("Connection closed.\n");
    return 0;
}
