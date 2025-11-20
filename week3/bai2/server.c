#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#pragma comment(lib, "ws2_32.lib")

static void log_client_connection(const char* action, const struct sockaddr_in* client) {
    char* ip_str = inet_ntoa(client->sin_addr);
    printf("[%s] %s:%d\n", action, ip_str, ntohs(client->sin_port));
}

static int is_valid_ipv4(const char* input) {
    return inet_addr(input) != INADDR_NONE;
}

static int looks_like_ip_but_invalid(const char* input) {
    // Check if string contains only digits and dots but is not valid IP
    for (int i = 0; input[i]; i++) {
        if (!isdigit(input[i]) && input[i] != '.') {
            return 0;
        }
    }
    return !is_valid_ipv4(input);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        return 1;
    }

    setvbuf(stdout, NULL, _IONBF, 0);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((u_short)atoi(argv[1]));

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("DNS Server listening on 0.0.0.0:%s\n", argv[1]);

    char query[1024];
    char response[8192];
    
    while (1) {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        int bytes_received = recvfrom(sock, query, sizeof(query)-1, 0, 
                                     (struct sockaddr*)&client_addr, &client_len);
        if (bytes_received <= 0) continue;
        query[bytes_received] = '\0';

        log_client_connection("RECV", &client_addr);
        printf("  query=\"%s\"\n", query);

        response[0] = '\0';

        if (query[0] == '\0') {
            // Skip empty queries
            continue;
        } else if (is_valid_ipv4(query)) {
            // Reverse DNS lookup: IP -> hostname
            struct in_addr ip_addr;
            ip_addr.s_addr = inet_addr(query);
            
            struct hostent* host_info = gethostbyaddr((const char*)&ip_addr, sizeof(ip_addr), AF_INET);
            if (!host_info || !host_info->h_name) {
                snprintf(response, sizeof(response), "Not found information");
            } else {
                // Build response with official name and aliases
                int pos = snprintf(response, sizeof(response), "Official name: %s\nAlias name:\n", host_info->h_name);
                
                if (host_info->h_aliases) {
                    for (char** alias = host_info->h_aliases; *alias && pos < sizeof(response) - 100; ++alias) {
                        pos += snprintf(response + pos, sizeof(response) - pos, "%s\n", *alias);
                    }
                }
            }
        } else if (looks_like_ip_but_invalid(query)) {
            // Invalid IP format
            snprintf(response, sizeof(response), "IP Address is invalid");
        } else {
            // Forward DNS lookup: domain -> IP addresses
            struct hostent* host_info = gethostbyname(query);
            if (!host_info || host_info->h_addrtype != AF_INET || !host_info->h_addr_list || !host_info->h_addr_list[0]) {
                snprintf(response, sizeof(response), "Not found information");
            } else {
                // Build response with official IP and alias IPs
                char first_ip[INET_ADDRSTRLEN] = {0};
                char alias_ips[4096] = {0};
                
                int idx = 0;
                for (char** addr_ptr = host_info->h_addr_list; *addr_ptr && idx < 10; ++addr_ptr, ++idx) {
                    struct in_addr addr;
                    memcpy(&addr, *addr_ptr, sizeof(addr));
                    char* ip_str = inet_ntoa(addr);
                    
                    if (idx == 0) {
                        strncpy(first_ip, ip_str, sizeof(first_ip)-1);
                    } else {
                        int alias_len = strlen(alias_ips);
                        if (alias_len + strlen(ip_str) + 2 < sizeof(alias_ips)) {
                            snprintf(alias_ips + alias_len, sizeof(alias_ips) - alias_len, "%s\n", ip_str);
                        }
                    }
                }
                
                if (first_ip[0] == '\0') {
                    snprintf(response, sizeof(response), "Not found information");
                } else {
                    snprintf(response, sizeof(response), "Official IP: %s\nAlias IP:\n%s", first_ip, alias_ips);
                }
            }
        }

        sendto(sock, response, (int)strlen(response), 0, 
               (struct sockaddr*)&client_addr, client_len);
        printf("SEND:\n%s\n", response);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
