#include "inc/syscalls.h"
#include "inc/common.h"
#include "lib.h"

#include "app_names.h"
extern char* _app_names[];


#define SLEEP_TIME 300000000

#define MAX_INPUT 128

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

int strncmp(const char *s1, const char *s2, unsigned int n) {
    for (unsigned int i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == '\0' || s2[i] == '\0') {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
    }
    return 0;
}


char input_buf[MAX_INPUT];


void main() {
    static char input_buf[MAX_INPUT];

    printf("SHELL Started registered apps are:\n");
    
    for (int i = 0; i < APP_COUNT; i++) {
        printf("Available runnable %d: %s\n", i, _app_names[i]);
    }

    while (1){
        printf("user> ");
        
        int len = get_string(input_buf, MAX_INPUT);
        
        printf("\nYou typed: %s\n", input_buf);
        sleep(SLEEP_TIME);
        printf("Mock done something..\n");

        if (strncmp(input_buf, "q", 2) == 0) {
            printf("Bye!\n");
            break;
        }
    }
}