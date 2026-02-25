#include "direct_printf.h"
#include "lib.h"
#include "inc/syscalls.h"
static const uint8_t digits[] = "0123456789ABCDEF";

int console_write_direct(char *buf, int len){
    return syscall(SYS_CONSOLE_PUT_DIRECT, (int)buf, len, 0, 0);
}

void own_putchar(char ch){
    console_write_direct(&ch, 1);
}

static void printint(int64_t xx, int32_t base, int32_t sgn) {
    uint8_t buf[32];
    int32_t i = 0;
    uint64_t x;

    if (sgn && xx < 0) {
        x = -xx;
        own_putchar('-');
    } else {
        x = xx;
    }

    if (x == 0) {
        own_putchar('0');
        return;
    }

    while (x != 0) {
        buf[i++] = digits[x % base];
        x /= base;
    }

    while (--i >= 0)
        own_putchar(buf[i]);
}

static void printptr(uintptr_t x) {
    int32_t i;
    own_putchar('0');
    own_putchar('x');
    for (i = (sizeof(uintptr_t) * 8) - 4; i >= 0; i -= 4) {
        own_putchar(digits[(x >> i) & 0xF]);
    }
}
void direct_printf(const char *fmt, ...) {
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
                own_putchar(c0);
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
                own_putchar(va_arg(ap, int32_t));
            } else if (c0 == 's') {
                if ((s = va_arg(ap, uint8_t *)) == 0)
                    s = (const uint8_t *)"(null)";
                for (; *s; s++)
                    own_putchar(*s);
            } else if (c0 == '%') {
                own_putchar('%');
            } else {
                own_putchar('%');
                own_putchar(c0);
            }
            state = 0;
        }
    }
    va_end(ap);
}