#define _WIN32_WINNT 0x0600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

// Hằng số compile-time cho kích thước mảng địa chỉ (tránh lỗi IntelliSense MSVC về VLA)
#ifndef MAXA
#define MAXA 128
#endif

/* --------------------------------------------------
   Compatibility wrappers for inet_pton / inet_ntop
   (MinGW old versions may miss them)
   -------------------------------------------------- */
#ifndef HAVE_INET_PTON
int inet_pton_win(int af, const char *src, void *dst) {
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;
    hints.ai_flags = AI_NUMERICHOST;
    if (getaddrinfo(src, NULL, &hints, &res) != 0) return 0;
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
    struct sockaddr_storage ss; DWORD len = (DWORD)size;
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
    if (WSAAddressToStringA((struct sockaddr*)&ss, (DWORD)(af==AF_INET?sizeof(struct sockaddr_in):sizeof(struct sockaddr_in6)), NULL, dst, &len) != 0)
        return NULL;
    return dst;
}
#define inet_ntop(a,b,c,d) inet_ntop_win(a,b,c,d)
#endif

/* --------------------------------------------------
   Utility: trim, split, timestamp, logging
   -------------------------------------------------- */
static void trim(char *s) {
    size_t len = strlen(s);
    while (len && (s[len-1]=='\n' || s[len-1]=='\r' || isspace((unsigned char)s[len-1]))) { s[--len]='\0'; }
    char *p = s; while (*p && isspace((unsigned char)*p)) p++; if (p!=s) memmove(s,p,strlen(p)+1);
}

static void log_line(FILE *logf, const char *fmt, ...) {
    if(!logf) return;
    va_list ap; va_start(ap, fmt);
    vfprintf(logf, fmt, ap);
    va_end(ap);
    fflush(logf);
}

static double now_ms() {
    static LARGE_INTEGER freq; static int init=0; LARGE_INTEGER c;
    if(!init){ QueryPerformanceFrequency(&freq); init=1; }
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1000.0 / (double)freq.QuadPart;
}

/* --------------------------------------------------
   IP format / validation helpers
   -------------------------------------------------- */
int parse_ipv4(const char *s, unsigned char bytes[4]) {
    if (!s || !*s) return 0;
    int count = 0;
    const char *p = s;
    while (*p) {
        if (count == 4) return 0; // quá 4 nhóm
        // Parse một số 0..255
        if (!isdigit((unsigned char)*p)) return 0; // phải bắt đầu bằng digit
        unsigned int val = 0; int digits = 0;
        while (*p && isdigit((unsigned char)*p)) {
            val = val*10 + (unsigned)(*p - '0');
            if (val > 255) return 0; // vượt 255
            p++; digits++;
        }
        if (digits == 0) return 0;
        bytes[count++] = (unsigned char)val;
        if (*p == '.') { // còn nhóm tiếp theo
            p++; // bỏ dấu '.'
            if (!*p) return 0; // không được kết thúc bằng '.'
            continue;
        } else if (*p == '\0') {
            break; // hết chuỗi
        } else {
            return 0; // ký tự lạ
        }
    }
    return count == 4 && *p == '\0';
}

int is_valid_ipv4(const char *s) {
    unsigned char b[4];
    return parse_ipv4(s,b);
}

int is_valid_ipv6(const char *s) {
    if (!s || !*s) return 0;
    struct addrinfo hints,*res=NULL; memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET6; hints.ai_flags = AI_NUMERICHOST;
    if (getaddrinfo(s,NULL,&hints,&res)==0) { freeaddrinfo(res); return 1; }
    return 0;
}

int is_invalid_ip_format(const char *s) {
    if (strchr(s,':')) return !is_valid_ipv6(s);
    if (strchr(s,'.')) {
        int has_alpha=0; for (const char *p=s; *p; ++p) if (isalpha((unsigned char)*p)) { has_alpha=1; break; }
        if (has_alpha) return 0; // domain
        return !is_valid_ipv4(s);
    }
    return 0;
}

/* --------------------------------------------------
   Special IP detection (loopback/private/link-local/multicast...)
   -------------------------------------------------- */
int is_special_ipv4(const unsigned char b[4]) {
    if (b[0]==127) return 1; // loopback
    if (b[0]==10) return 1; // private
    if (b[0]==172 && (b[1]>=16 && b[1]<=31)) return 1; // private
    if (b[0]==192 && b[1]==168) return 1; // private
    if (b[0]==169 && b[1]==254) return 1; // link local
    if (b[0]>=224 && b[0]<=239) return 1; // multicast
    if (b[0]==255 && b[1]==255 && b[2]==255 && b[3]==255) return 1; // broadcast
    if (b[0]==0) return 1; // unspecified
    return 0;
}

int is_special_ipv6(const struct in6_addr *addr) {
    const unsigned char *b = (const unsigned char*)addr->s6_addr;
    // ::
    int allzero=1; for(int i=0;i<16;i++) if(b[i]!=0){allzero=0;break;}
    if(allzero) return 1; // unspecified
    // ::1
    int loop=1; for(int i=0;i<15;i++) if(b[i]!=0){loop=0;break;} if(loop && b[15]==1) return 1;
    // fe80::/10 link local
    if (b[0]==0xfe && (b[1]&0xc0)==0x80) return 1;
    // fc00::/7 unique local (fc or fd)
    if ((b[0]&0xfe)==0xfc) return 1;
    // ff00::/8 multicast
    if (b[0]==0xff) return 1;
    return 0;
}

/* --------------------------------------------------
   Reverse DNS: IP -> Hostname + Aliases
   -------------------------------------------------- */
void reverse_lookup(const char *ip, FILE *logf) {
    double t0 = now_ms();
    char host[NI_MAXHOST];
    int ret;
    int is_ipv4 = is_valid_ipv4(ip);
    int is_ipv6_addr = !is_ipv4 && is_valid_ipv6(ip);

    printf("Query: %s\n", ip);
    log_line(logf, "Query: %s\n", ip);

    if (!is_ipv4 && !is_ipv6_addr) {
        printf("Invalid address\n\n");
        log_line(logf, "Invalid address\n\n");
        return;
    }

    // Special IP warning
    if (is_ipv4) {
        unsigned char b[4]; parse_ipv4(ip,b);
        if (is_special_ipv4(b)) {
            printf("special IP address — may not have DNS record\n");
            log_line(logf, "special IP address — may not have DNS record\n");
        }
    } else {
        struct in6_addr a6; if (inet_pton(AF_INET6, ip, &a6)==1 && is_special_ipv6(&a6)) {
            printf("special IP address — may not have DNS record\n");
            log_line(logf, "special IP address — may not have DNS record\n");
        }
    }

    if (is_ipv4) {
        struct sockaddr_in sa; memset(&sa,0,sizeof(sa)); sa.sin_family=AF_INET; sa.sin_addr.s_addr=inet_addr(ip);
        ret = getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host), NULL, 0, NI_NAMEREQD);
    } else {
        struct sockaddr_in6 sa6; memset(&sa6,0,sizeof(sa6)); sa6.sin6_family=AF_INET6; inet_pton(AF_INET6, ip, &sa6.sin6_addr);
        ret = getnameinfo((struct sockaddr*)&sa6, sizeof(sa6), host, sizeof(host), NULL, 0, NI_NAMEREQD);
    }

    if (ret != 0) {
        printf("Not found information\n");
        log_line(logf, "Not found information\n");
    } else {
        printf("Official name: %s\n", host);
        log_line(logf, "Official name: %s\n", host);
        printf("Alias name:\n");
        log_line(logf, "Alias name:\n");
        struct hostent *he = gethostbyname(host);
        int printed=0;
        if (he && he->h_aliases) {
            for (int i=0; he->h_aliases[i]; ++i) { printf("%s\n", he->h_aliases[i]); log_line(logf, "%s\n", he->h_aliases[i]); printed=1; }
        }
        if (!printed) { printf("%s\n", host); log_line(logf, "%s\n", host); }
    }
    double t1 = now_ms();
    printf("Query time: %.3f ms\n\n", t1 - t0);
    log_line(logf, "Query time: %.3f ms\n\n", t1 - t0);
}

/* --------------------------------------------------
   Forward DNS: Hostname -> IP list + CNAME + alias
   -------------------------------------------------- */
void forward_lookup(const char *name, FILE *logf) {
    if (!name || !*name) return; // guard against empty token
    double t0 = now_ms();
    struct addrinfo hints, *res=NULL, *p;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME; // yêu cầu canonical name

    printf("Query: %s\n", name);
    log_line(logf, "Query: %s\n", name);

    int r = getaddrinfo(name, NULL, &hints, &res);
    if (r != 0 || !res) {
        printf("Not found information\n\n");
        log_line(logf, "Not found information\n\n");
        return;
    }

    if (res->ai_canonname) {
        printf("CNAME: %s\n", res->ai_canonname);
        log_line(logf, "CNAME: %s\n", res->ai_canonname);
    }

    // Thu thập địa chỉ (dùng macro MAXA thay vì const int để MSVC coi là hằng compile-time)
    char v4[MAXA][INET_ADDRSTRLEN]; int n4=0;
    char v6[MAXA][INET6_ADDRSTRLEN]; int n6=0;
    for (p=res; p && (n4<MAXA && n6<MAXA); p=p->ai_next) {
        char buf[INET6_ADDRSTRLEN];
        if (p->ai_family==AF_INET) {
            struct sockaddr_in *sa=(struct sockaddr_in*)p->ai_addr;
            if (inet_ntop(AF_INET,&sa->sin_addr,buf,sizeof(buf))) {
                int dup=0; for(int i=0;i<n4;i++) if(strcmp(v4[i],buf)==0){dup=1;break;} if(!dup) strcpy(v4[n4++],buf);
            }
        } else if (p->ai_family==AF_INET6) {
            struct sockaddr_in6 *sa6=(struct sockaddr_in6*)p->ai_addr;
            if (inet_ntop(AF_INET6,&sa6->sin6_addr,buf,sizeof(buf))) {
                int dup=0; for(int i=0;i<n6;i++) if(strcmp(v6[i],buf)==0){dup=1;break;} if(!dup) strcpy(v6[n6++],buf);
            }
        }
    }

    if (n4==0 && n6==0) {
        printf("Not found information\n\n");
        log_line(logf, "Not found information\n\n");
        freeaddrinfo(res); return;
    }

    if (n4>0) { printf("Official IP: %s\n", v4[0]); log_line(logf, "Official IP: %s\n", v4[0]); }
    else { printf("Official IP: %s\n", v6[0]); log_line(logf, "Official IP: %s\n", v6[0]); }

    printf("Alias IP:\n"); log_line(logf, "Alias IP:\n");
    for (int i=0;i<n6;i++) { // ưu tiên IPv6 trong alias (trừ khi official chính là IPv6 đầu tiên)
        if (!(n4==0 && i==0)) { printf("%s\n", v6[i]); log_line(logf, "%s\n", v6[i]); }
    }
    for (int i=(n4>0?1:0); i<n4; i++) { printf("%s\n", v4[i]); log_line(logf, "%s\n", v4[i]); }

    double t1 = now_ms();
    printf("Query time: %.3f ms\n\n", t1 - t0);
    log_line(logf, "Query time: %.3f ms\n\n", t1 - t0);
    freeaddrinfo(res);
}

/* --------------------------------------------------
   Dispatcher
   -------------------------------------------------- */
void resolve_token(const char *token, FILE *logf) {
    if (!token) return;
    if (!*token) return; // ignore empty token explicitly
    // Bộ lọc ký tự bất thường: chỉ cho phép [A-Za-z0-9.:-_] và dấu '-'
    int suspicious = 0; int all_punct = 1; int has_alnum = 0;
    for (const unsigned char *p=(const unsigned char*)token; *p; ++p) {
        if (isalnum(*p)) { has_alnum = 1; all_punct = 0; continue; }
        if (*p=='.' || *p==':' || *p=='-' || *p=='_' ) continue;
        // Các ký tự whitespace/control hoặc ký tự extended coi là nghi ngờ
        suspicious = 1; break;
    }
    // Nếu toàn dấu chấm hoặc toàn dấu '-' cũng coi là nghi ngờ
    if (!suspicious && !has_alnum) suspicious = 1;
    if (suspicious) {
        printf("Query: %s\nIgnored suspicious token\n\n", token);
        log_line(logf, "Query: %s\nIgnored suspicious token\n\n", token);
        return;
    }
    if (is_invalid_ip_format(token)) {
        printf("Query: %s\nInvalid address\n\n", token);
        log_line(logf, "Query: %s\nInvalid address\n\n", token);
        return;
    }
    if (is_valid_ipv4(token) || is_valid_ipv6(token)) reverse_lookup(token, logf);
    else forward_lookup(token, logf);
}

/* --------------------------------------------------
   Batch mode from file
   -------------------------------------------------- */
void batch_mode(const char *filename, FILE *logf) {
    FILE *f = fopen(filename, "r");
    if (!f) { fprintf(stderr, "Cannot open file: %s\n", filename); return; }
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (!*line) continue;
        // Tách nhiều token trên 1 dòng
        char *tok = strtok(line, " \t");
        while (tok) { resolve_token(tok, logf); tok = strtok(NULL, " \t"); }
    }
    fclose(f);
}

/* --------------------------------------------------
   Main: interactive + batch
   -------------------------------------------------- */
int main(int argc, char *argv[]) {
    WSADATA w; if (WSAStartup(MAKEWORD(2,2), &w)!=0) { fprintf(stderr, "WSAStartup failed\n"); return 1; }

    // Đảm bảo prompt hiển thị ngay (unbuffered stdout)
    setvbuf(stdout, NULL, _IONBF, 0);

    FILE *logf = fopen("resolver2.log", "a");
    if (logf) {
        time_t tt = time(NULL); struct tm *tmv = localtime(&tt);
        char ts[64]; strftime(ts,sizeof(ts),"%Y-%m-%d %H:%M:%S", tmv);
        fprintf(logf, "===== Session start %s =====\n", ts);
    }

    if (argc > 1) {
        // Nếu tham số đầu tiên là file tồn tại -> batch mode
        FILE *test = fopen(argv[1], "r");
        if (test) { fclose(test); batch_mode(argv[1], logf); }
        else {
            // Xử lý từng tham số như một token
            for (int i=1; i<argc; ++i) resolve_token(argv[i], logf);
        }
        if (logf) { fprintf(logf, "===== Session end =====\n"); fclose(logf); }
        WSACleanup();
        return 0;
    }

    // Interactive mode
    char line[1024];
    printf("Enter domain/IP (blank line to quit). You can input multiple tokens separated by spaces.\n");
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break; // EOF
        trim(line);
        if (!*line) break; // blank => stop program per spec
        // Bảo vệ: nếu sau trim vẫn rỗng (trường hợp ký tự control) bỏ qua
        if (line[0] == '\0') continue;
        char *tok = strtok(line, " \t");
        while (tok) {
            // Nếu token rỗng (bảo vệ bổ sung)
            if (*tok == '\0') { tok = strtok(NULL, " \t"); continue; }
            resolve_token(tok, logf);
            tok = strtok(NULL, " \t");
        }
    }

    if (logf) { fprintf(logf, "===== Session end =====\n"); fclose(logf); }
    WSACleanup();
    return 0;
}
