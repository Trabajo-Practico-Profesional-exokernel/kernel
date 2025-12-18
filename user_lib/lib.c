#include "lib.h"

int get_string(char *buf, int max_len) {
    int i = 0;
    while (i < max_len - 1) {
        int ch = getchar();

        // Echo the character back (optional, typical shell behavior)
        putchar(ch);

        // Check for Enter (carriage return)
        if (ch == '\r') {
            break;
        }

        buf[i++] = ch;
    }

    buf[i] = '\0';  // Null-terminate the string
    return i;       // Return length of string
}
