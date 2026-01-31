#include "types.h"
#include "defs.h"
#include "constants.h"
#include "stdio.h"

extern void putchar(uint8_t ch);

static const uint8_t digits[] = "0123456789ABCDEF";

static int32_t string_print_num(uint8_t *buf, size_t size, size_t *pos, int64_t value, int32_t base, bool sign) {
    uint8_t tmp[64];
    int32_t i = 0;
    uint64_t uval;

    if (sign && value < 0) {
        uval = (uint64_t)(value * -1);
        if (*pos < size - 1) buf[(*pos)++] = '-';
    } else {
        uval = (uint64_t)value;
    }

    if (uval == 0) {
        tmp[i++] = '0';
    } else {
        while (uval > 0) {
            int32_t digit = uval % base;
            tmp[i++] = digits[digit];
            uval /= base;
        }
    }

    int32_t chars_written = 0;
    while (i > 0) {
        i--;
        if (*pos < size - 1) {
            buf[(*pos)++] = tmp[i];
            chars_written++;
        }
    }
    return chars_written;
}


int32_t vsnprintf(uint8_t *buf, size_t size, const uint8_t *fmt, va_list args) {
    size_t pos = 0;

    if (size == 0) return 0;

    for (const uint8_t *p = fmt; *p != '\0'; p++) {
        if (*p != '%') {
            if (pos < size - 1) buf[pos++] = *p;
            continue;
        }

        p++;

        switch (*p) {
            case 'd':
            case 'i': {
                int32_t val = va_arg(args, int32_t);
                string_print_num(buf, size, &pos, val, 10, true);
                break;
            }
            case 'u': {
                uint32_t val = va_arg(args, uint32_t);
                string_print_num(buf, size, &pos, val, 10, false);
                break;
            }
            case 'x': 
            case 'p': {
                uintptr_t val = va_arg(args, uintptr_t);
                string_print_num(buf, size, &pos, val, 16, false);
                break;
            }
            case 's': {
                const uint8_t *s = va_arg(args, const uint8_t *);
                if (!s) s = (const uint8_t *)"(null)";
                while (*s) {
                    if (pos < size - 1) buf[pos++] = *s;
                    s++;
                }
                break;
            }
            case 'c': {
                int32_t c = va_arg(args, int32_t);
                if (pos < size - 1) buf[pos++] = (uint8_t)c;
                break;
            }
            case '%': {
                if (pos < size - 1) buf[pos++] = '%';
                break;
            }
            default:
                if (pos < size - 1) buf[pos++] = '%';
                if (pos < size - 1) buf[pos++] = *p;
                break;
        }
    }

    buf[pos] = '\0';
    return (int32_t)pos;
}

int32_t snprintf(uint8_t *buf, size_t size, const uint8_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int32_t ret = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return ret;
}

static void printint(int64_t xx, int32_t base, int32_t sgn) {
    uint8_t buf[32];
    int32_t i = 0;
    uint64_t x;

    if (sgn && xx < 0) {
        x = -xx;
        putchar('-');
    } else {
        x = xx;
    }

    if (x == 0) {
        putchar('0');
        return;
    }

    while (x != 0) {
        buf[i++] = digits[x % base];
        x /= base;
    }

    while (--i >= 0)
        putchar(buf[i]);
}

static void printptr(uintptr_t x) {
    int32_t i;
    putchar('0');
    putchar('x');
    for (i = (sizeof(uintptr_t) * 8) - 4; i >= 0; i -= 4) {
        putchar(digits[(x >> i) & 0xF]);
    }
}

void printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    const uint8_t *s;
    // int32_t c0, c1, c2, i, state; // Not used c2...
    int32_t c0, c1, i, state;

    state = 0;
    for (i = 0; fmt[i]; i++) {
        c0 = fmt[i] & 0xff;
        if (state == 0) {
            if (c0 == '%') {
                state = '%';
            } else {
                putchar(c0);
            }
        } else if (state == '%') {
            c1 = fmt[i + 1] & 0xff;
            // c2 = fmt[i + 2] & 0xff;

            if (c0 == 'd') {
                printint(va_arg(ap, int32_t), 10, 1);
            } else if (c0 == 'l' && c1 == 'd') {
                printint(va_arg(ap, int64_t), 10, 1);
                i += 1;
            } else if (c0 == 'u') {
                printint(va_arg(ap, uint32_t), 10, 0);
            } else if (c0 == 'l' && c1 == 'u') {
                printint(va_arg(ap, uint64_t), 10, 0);
                i += 1;
            } else if (c0 == 'x') {
                printint(va_arg(ap, uint32_t), 16, 0);
            } else if (c0 == 'l' && c1 == 'x') {
                printint(va_arg(ap, uint64_t), 16, 0);
                i += 1;
            } else if (c0 == 'p') {
                printptr(va_arg(ap, uintptr_t));
            } else if (c0 == 'c') {
                putchar(va_arg(ap, int32_t));
            } else if (c0 == 's') {
                if ((s = va_arg(ap, uint8_t *)) == 0)
                    s = (const uint8_t *)"(null)";
                for (; *s; s++)
                    putchar(*s);
            } else if (c0 == '%') {
                putchar('%');
            } else {
                putchar('%');
                putchar(c0);
            }
            state = 0;
        }
    }
    va_end(ap);
}

uint64_t __udivdi3(uint64_t n, uint64_t d) {
    uint64_t q = 0, r = 0;
    for (int i = 63; i >= 0; i--) {
        r <<= 1;
        r |= (n >> i) & 1;
        if (r >= d) {
            r -= d;
            q |= (1ULL << i);
        }
    }
    return q;
}

uint64_t __umoddi3(uint64_t n, uint64_t d) {
    uint64_t r = 0;
    for (int i = 63; i >= 0; i--) {
        r <<= 1;
        r |= (n >> i) & 1;
        if (r >= d) {
            r -= d;
        }
    }
    return r;
}
