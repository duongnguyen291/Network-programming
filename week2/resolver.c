// Yêu cầu Windows Vista trở lên để dùng đầy đủ IPv6 API (inet_pton / getaddrinfo)
#define _WIN32_WINNT 0x0600

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// Bổ sung hằng số compile-time để tránh lỗi IntelliSense (MSVC không coi 'const int' nội bộ hàm là hằng thời điểm biên dịch)
#ifndef MAX_ADDR
#define MAX_ADDR 64
#endif

/* =============================================
   COMPAT WRAPPERS (MinGW có thể thiếu inet_pton / inet_ntop)
   ============================================= */
#ifndef HAVE_INET_PTON
int inet_pton_win(int af, const char *src, void *dst) {
    struct sockaddr_storage ss;
    char txt[INET6_ADDRSTRLEN + 8];
    memset(&ss, 0, sizeof(ss));
    snprintf(txt, sizeof(txt), "%s", src);
    WSADATA w; // đảm bảo WSAStartup đã gọi bên ngoài
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;
    hints.ai_flags = AI_NUMERICHOST;
    if (getaddrinfo(txt, NULL, &hints, &res) != 0) return 0;
    if (af == AF_INET) {
        memcpy(dst, &((struct sockaddr_in*)res->ai_addr)->sin_addr, sizeof(struct in_addr));
    } else if (af == AF_INET6) {
        memcpy(dst, &((struct sockaddr_in6*)res->ai_addr)->sin6_addr, sizeof(struct in6_addr));
    }
    freeaddrinfo(res);
    return 1;
}
#define inet_pton(a,b,c) inet_pton_win(a,b,c)
#endif

#ifndef HAVE_INET_NTOP
const char *inet_ntop_win(int af, const void *src, char *dst, size_t size) {
    struct sockaddr_storage ss; DWORD len = (DWORD)size; int r;
    memset(&ss, 0, sizeof(ss));
    if (af == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in*)&ss;
        sa->sin_family = AF_INET;
        memcpy(&sa->sin_addr, src, sizeof(struct in_addr));
    } else if (af == AF_INET6) {
        struct sockaddr_in6 *sa6 = (struct sockaddr_in6*)&ss;
        sa6->sin6_family = AF_INET6;
        memcpy(&sa6->sin6_addr, src, sizeof(struct in6_addr));
    } else return NULL;
    r = WSAAddressToStringA((struct sockaddr*)&ss, (DWORD)(af==AF_INET?sizeof(struct sockaddr_in):sizeof(struct sockaddr_in6)), NULL, dst, &len);
    if (r != 0) return NULL;
    return dst;
}
#define inet_ntop(a,b,c,d) inet_ntop_win(a,b,c,d)
#endif

/* =============================================
   HÀM HỖ TRỢ KIỂM TRA / PHÂN LOẠI THAM SỐ ĐẦU VÀO
   ============================================= */

// Kiểm tra IPv4 dạng n.n.n.n (mỗi n 0..255, không cho thiếu phần)
int parse_ipv4(const char *s, unsigned char bytes[4]) {
    if (!s || !*s) return 0;
    char copy[64];
    strncpy(copy, s, sizeof(copy) - 1);
    copy[sizeof(copy)-1] = '\0';
    char *token = strtok(copy, ".");
    int count = 0;
    while (token) {
        if (count >= 4) return 0; // quá 4 phần
        if (*token == '\0') return 0; // phần rỗng
        char *endp; long v = strtol(token, &endp, 10);
        if (*endp != '\0') return 0; // có ký tự không phải số => không phải IPv4 thuần
        if (v < 0 || v > 255) return 0;
        bytes[count++] = (unsigned char)v;
        token = strtok(NULL, ".");
    }
    return count == 4;
}

int is_valid_ipv4(const char *s) {
    unsigned char b[4];
    if (!parse_ipv4(s, b)) return 0;
    return 1; // hợp lệ
}

int is_valid_ipv6(const char *s) {
    // Dùng getaddrinfo với cờ AI_NUMERICHOST để xác thực IPv6 thuần cú pháp
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_flags = AI_NUMERICHOST;
    if (getaddrinfo(s, NULL, &hints, &res) == 0) {
        freeaddrinfo(res);
        return 1;
    }
    return 0;
}

int is_invalid_ip_format(const char *s) {
    // Có dấu ':' => có thể là IPv6 numeric. Nếu không hợp lệ => invalid.
    if (strchr(s, ':')) return !is_valid_ipv6(s);
    // Có dấu '.' => phân biệt IPv4 hay domain. Nếu có chữ cái -> domain => không invalid
    if (strchr(s, '.')) {
        int has_alpha = 0;
        for (const char *p = s; *p; ++p) {
            if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) { has_alpha = 1; break; }
        }
        if (has_alpha) return 0; // domain chứa chấm
        // toàn số và dấu '.' => phải là IPv4 hợp lệ, nếu không => invalid
        return !is_valid_ipv4(s);
    }
    return 0; // domain thuần chữ/số không dấu chấm => không phải IP => không invalid
}

/* =============================================
   HÀM RESOLVE IP -> HOSTNAME (REVERSE DNS)
   ============================================= */
void resolve_ip_to_hostname(const char *ip_str) {
    char host[NI_MAXHOST];
    int ret;

    if (is_valid_ipv4(ip_str)) {
        struct sockaddr_in sa; memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr(ip_str);
        ret = getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host), NULL, 0, NI_NAMEREQD);
        if (ret == 0) {
            printf("Official name: %s\n", host);
            // Lấy thêm alias bằng gethostbyname (chỉ IPv4)
            struct hostent *he = gethostbyname(host);
            printf("Alias name:\n");
            int printed = 0;
            if (he && he->h_aliases) {
                for (int i = 0; he->h_aliases[i]; ++i) {
                    printf("%s\n", he->h_aliases[i]);
                    printed = 1;
                }
            }
            if (!printed) {
                // Để gần giống ví dụ (in lại chính tên nếu không có alias thực)
                printf("%s\n", host);
            }
        } else {
            printf("Not found information\n");
        }
        return;
    }

    if (is_valid_ipv6(ip_str)) {
        struct sockaddr_in6 sa6; memset(&sa6, 0, sizeof(sa6));
        sa6.sin6_family = AF_INET6;
        // Chuyển chuỗi sang nhị phân
        if (inet_pton(AF_INET6, ip_str, &sa6.sin6_addr) == 1) {
            ret = getnameinfo((struct sockaddr*)&sa6, sizeof(sa6), host, sizeof(host), NULL, 0, NI_NAMEREQD);
            if (ret == 0) {
                printf("Official name: %s\n", host);
                printf("Alias name:\n%s\n", host); // Thường alias IPv6 ít có, in lại
            } else {
                printf("Not found information\n");
            }
        } else {
            printf("Invalid address\n");
        }
        return;
    }

    printf("Invalid address\n");
}

/* =============================================
   HÀM RESOLVE HOSTNAME -> LIST IP (FORWARD DNS)
   ============================================= */
void resolve_hostname_to_ip(const char *hostname) {
    struct addrinfo hints, *res = NULL, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // IPv4 & IPv6
    hints.ai_socktype = SOCK_STREAM;  // Không thực sự tạo socket, chỉ để lọc

    int ret = getaddrinfo(hostname, NULL, &hints, &res);
    if (ret != 0 || !res) {
        printf("Not found information\n");
        return;
    }

    // Lưu tất cả địa chỉ vào mảng tạm để phân loại
    char ipv4_list[MAX_ADDR][INET_ADDRSTRLEN];
    char ipv6_list[MAX_ADDR][INET6_ADDRSTRLEN];
    int v4_count = 0, v6_count = 0;

    for (p = res; p && (v4_count < MAX_ADDR || v6_count < MAX_ADDR); p = p->ai_next) {
        void *addr_ptr = NULL;
        char buf[INET6_ADDRSTRLEN];
        if (p->ai_family == AF_INET) {
            addr_ptr = &((struct sockaddr_in*)p->ai_addr)->sin_addr;
            if (inet_ntop(AF_INET, addr_ptr, buf, sizeof(buf))) {
                // Tránh trùng lặp cơ bản
                int dup = 0; for (int i=0;i<v4_count;i++) if (strcmp(ipv4_list[i], buf)==0) {dup=1;break;}
                if (!dup && v4_count < MAX_ADDR) {
                    strcpy(ipv4_list[v4_count++], buf);
                }
            }
        } else if (p->ai_family == AF_INET6) {
            addr_ptr = &((struct sockaddr_in6*)p->ai_addr)->sin6_addr;
            if (inet_ntop(AF_INET6, addr_ptr, buf, sizeof(buf))) {
                int dup = 0; for (int i=0;i<v6_count;i++) if (strcmp(ipv6_list[i], buf)==0) {dup=1;break;}
                if (!dup && v6_count < MAX_ADDR) {
                    strcpy(ipv6_list[v6_count++], buf);
                }
            }
        }
    }

    if (v4_count == 0 && v6_count == 0) {
        printf("Not found information\n");
        freeaddrinfo(res);
        return;
    }

    // Chọn Official IP: ưu tiên IPv4 đầu tiên, nếu không có thì IPv6
    if (v4_count > 0) {
        printf("Official IP: %s\n", ipv4_list[0]);
    } else {
        printf("Official IP: %s\n", ipv6_list[0]);
    }

    printf("Alias IP:\n");
    // In các IPv6 trước (phù hợp ví dụ có IPv6 đứng đầu alias) rồi các IPv4 còn lại
    for (int i = 0; i < v6_count; ++i) {
        // Nếu IPv6 này trùng với official (trường hợp official là IPv6)
        if (!(v4_count == 0 && i == 0))
            printf("%s\n", ipv6_list[i]);
    }
    for (int i = 1; i < v4_count; ++i) { // bắt đầu từ 1 để bỏ official nếu official là IPv4
        printf("%s\n", ipv4_list[i]);
    }

    freeaddrinfo(res);
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    int r = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (r != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", r);
        return 1;
    }

    if (argc != 2) {
        printf("Usage: %s <domain_name_or_ip_address>\n", argv[0]);
        WSACleanup();
        return 1;
    }

    const char *param = argv[1];

    // Phát hiện format IP không hợp lệ trước
    if (is_invalid_ip_format(param)) {
        printf("Invalid address\n");
        WSACleanup();
        return 1;
    }

    if (is_valid_ipv4(param) || is_valid_ipv6(param)) {
        resolve_ip_to_hostname(param);
    } else {
        resolve_hostname_to_ip(param);
    }

    WSACleanup();
    return 0;
}