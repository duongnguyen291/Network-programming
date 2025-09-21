#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
 
/* --------- small string utils ---------- */
static void trim(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t i = 0, j = len;
 
    while (i < len && isspace((unsigned char)s[i])) i++;
    while (j > i && isspace((unsigned char)s[j - 1])) j--;
 
    if (i > 0) memmove(s, s + i, j - i);
    s[j - i] = '\0';
}
 
static void to_upper(char *s) {
    if (!s) return;
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}
 
/* Keep only [A-Za-z0-9] and uppercase the result */
static void compact_alnum_upper(char *dst, const char *src, size_t dstsz) {
    size_t k = 0;
    for (size_t i = 0; src[i] != '\0' && k + 1 < dstsz; ++i) {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c)) {
            dst[k++] = (char)toupper(c);
        }
    }
    dst[k] = '\0';
}
 
/* --------- domain mapping ---------- */
static const char* material_from_digit(char d) {
    switch (d) {
        case '1': return "jets";
        case '2': return "fog";
        case '3': return "foam";
        case '4': return "dry agent";
        default : return "unknown";
    }
}
 
/* Reactivity: 'can be violently reactive' if letter is in {P,S,W,Y,Z} */
static bool is_violently_reactive(char c) {
    switch (c) {
        case 'P': case 'S': case 'W': case 'Y': case 'Z':
            return true;
        default:
            return false;
    }
}
 
/* Protection:
   - P, R, W, X -> Full protective clothing
   - S, T, Y, Z -> BA for fire only (to match the sample output)
*/
static const char* protection_for_letter(char c) {
    switch (c) {
        case 'P': case 'R': case 'W': case 'X':
            return "full protective clothing must be worn";
        case 'S': case 'T': case 'Y': case 'Z':
            return "breathing apparatus, protective gloves for fire only";
        default:
            return "not specified";
    }
}
 
/* Containment:
   - P, R -> Dilute
   - W, X -> Contain
   - S, T, Y, Z -> depends on reverse coloured? yes -> Dilute, no -> Contain
*/
static const char* containment_for_letter(char c, bool reverse_coloured) {
    switch (c) {
        case 'P': case 'R':
            return "may be diluted and washed down the drain";
        case 'W': case 'X':
            return "a need to avoid spillages from entering drains or water courses";
        case 'S': case 'T': case 'Y': case 'Z':
            return reverse_coloured
                   ? "may be diluted and washed down the drain"
                   : "a need to avoid spillages from entering drains or water courses";
        default:
            return "not specified";
    }
}
 
/* yes/no parser that keeps politely bullying until valid */
static bool prompt_yes_no(const char *question) {
    char buf[64];
    for (;;) {
        printf("%s", question);
        if (!fgets(buf, sizeof(buf), stdin)) {
            /* EOF -> default to 'no' */
            return false;
        }
        trim(buf);
        to_upper(buf);
        if (strcmp(buf, "Y") == 0 || strcmp(buf, "YES") == 0 ||
            strcmp(buf, "T") == 0 || strcmp(buf, "TRUE") == 0 || strcmp(buf, "1") == 0) {
            return true;
        }
        if (strcmp(buf, "N") == 0 || strcmp(buf, "NO") == 0 ||
            strcmp(buf, "F") == 0 || strcmp(buf, "FALSE") == 0 || strcmp(buf, "0") == 0) {
            return false;
        }
        puts("Please answer yes or no (e.g., yes / no).");
    }
}
 
/* validate HAZCHEM: 2 or 3 chars, d in 1..4, letter in PRSTWXYZ, optional E */
static bool validate_hazchem(const char *code, char *d_out, char *l_out, bool *evac_out) {
    size_t n = strlen(code);
    if (n != 2 && n != 3) return false;
 
    char d = code[0];
    char l = code[1];
    if (d < '1' || d > '4') return false;
 
    const char *valid = "PRSTWXYZ";
    bool ok_letter = false;
    for (const char *p = valid; *p; ++p) if (*p == l) { ok_letter = true; break; }
    if (!ok_letter) return false;
 
    bool evac = false;
    if (n == 3) {
        if (code[2] != 'E') return false;
        evac = true;
    }
 
    if (d_out) *d_out = d;
    if (l_out) *l_out = l;
    if (evac_out) *evac_out = evac;
    return true;
}
 
int main(void) {
    char raw[256];
    char code[16];
 
    for (;;) {
        printf("Enter HAZCHEM code: ");
        if (!fgets(raw, sizeof(raw), stdin)) {
            puts("\nInput terminated.");
            return 1;
        }
        trim(raw);
        compact_alnum_upper(code, raw, sizeof(code));
 
        char d = 0, l = 0;
        bool evac = false;
        if (!validate_hazchem(code, &d, &l, &evac)) {
            puts("Invalid HAZCHEM code. Valid examples: 1P, 3SE, 4X, 2T...");
            continue;
        }
 
        bool need_colour = (l == 'S' || l == 'T' || l == 'Y' || l == 'Z');
        bool reverse_coloured = false;
        if (need_colour) {
            char q[128];
            snprintf(q, sizeof(q), "Is the %c reverse coloured? ", l);
            reverse_coloured = prompt_yes_no(q);
        }
 
        /* Output section */
        puts("\n***Emergency action advice***");
        printf("Material:    %s\n", material_from_digit(d));
        printf("Reactivity:  %s\n",
               is_violently_reactive(l) ? "can be violently reactive"
                                        : "not specified");
        printf("Protection:  %s\n", protection_for_letter(l));
        printf("Containment: %s\n", containment_for_letter(l, reverse_coloured));
        printf("Evacuation:  %s\n", evac ? "consider evacuation"
                                         : "no special evacuation advice");
        puts("*****************************\n");
 
        /* one-shot run; exit happy */
        return 0;
    }
}