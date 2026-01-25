#include "stdlib.h"

#include "types.h"
#include "string.h"
#include "stdlib.h"

void reverse(uint8_t *s) {
    int32_t i, j;
    uint8_t c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int32_t strtol(const uint8_t *s, uint8_t **endptr, int32_t base) {
    int32_t neg = 0;
    int32_t val = 0;

    while (*s == ' ' || *s == '\t')
        s++;

    if (*s == '+') {
        s++;
    } else if (*s == '-') {
        s++;
        neg = 1;
    }

    if ((base == 0 || base == 16) && (s[0] == '0' && s[1] == 'x')) {
        s += 2;
        base = 16;
    } else if (base == 0 && s[0] == '0') {
        s++;
        base = 8;
    } else if (base == 0) {
        base = 10;
    }

    while (1) {
        int32_t dig;

        if (*s >= '0' && *s <= '9')
            dig = *s - '0';
        else if (*s >= 'a' && *s <= 'z')
            dig = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'Z')
            dig = *s - 'A' + 10;
        else
            break;

        if (dig >= base)
            break;

        s++;
        val = (val * base) + dig;
    }

    if (endptr)
        *endptr = (uint8_t *)s;

    return (neg ? -val : val);
}

int32_t atoi(const uint8_t *s) {
    int32_t n = 0;
    int32_t sign = 1;
    
    if (*s == '-') {
        sign = -1;
        s++;
    }
    while (*s) {
        if (*s < '0' || *s > '9') break;
        n = n * 10 + (*s) - '0';
        s++;
    }
    return sign * n;
}


void itoa(uint32_t n, uint8_t *s) {
    int32_t i = 0;
    
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    
    s[i++] = '\0';
    reverse(s);
}

void itohex(uint32_t n, uint8_t *s) {
    int32_t i = 0;
    int32_t d;

    do {
        d = n % 16;
        if (d < 10) {
            s[i++] = d + '0';
        } else {
            s[i++] = d - 10 + 'a';
        }
    } while ((n /= 16) > 0);
    
    s[i++] = '\0';
    reverse(s);
}

void int_to_string(int32_t n, uint8_t *s) {
    int32_t i = 0;
    int32_t sign = n;
    
    if (sign < 0) n = -n;
    
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    
    if (sign < 0) s[i++] = '-';
    s[i] = '\0';

    int32_t j, k;
    uint8_t temp;
    for (j = 0, k = i - 1; j < k; j++, k--) {
        temp = s[j];
        s[j] = s[k];
        s[k] = temp;
    }
}
