 #include "utils.h"
 #include "std/string.h"
 #include "lib.h"

 #define KEY_ENTER 10
 #define KEY_BACKSPACE 127
 #define KEY_ESC 27
 #define MAX_HISTORY 10
 #define MAX_CMD_LEN 128
 
 static char history[MAX_HISTORY][MAX_CMD_LEN];
 
 void history_shift(char *new_cmd) {
     for (int i = MAX_HISTORY - 1; i > 1; i--) {
         strcpy(history[i], history[i - 1]);
     }
     
     strncpy(history[1], new_cmd, MAX_CMD_LEN - 1);
     history[1][MAX_CMD_LEN - 1] = '\0';
 
     history[0][0] = '\0';
 }
 
 int read_line(char *buf, int max_len) {
     int len = 0;
     int pos = 0;
     int hist_cursor = 0;
     int c;
 
     strcpy(buf, history[0]);
     len = strlen(buf);
     pos = len;
 
     printf("%s", buf);
 
     while (1) {
         c = getchar();
 
         if (c == KEY_ESC) {
             int next1 = getchar();
             int next2 = getchar();
             
             if (next1 == '[') {
                 if (next2 == 'A') { 
                     if (hist_cursor < MAX_HISTORY - 1 && history[hist_cursor + 1][0] != '\0') {
                         strncpy(history[hist_cursor], buf, MAX_CMD_LEN -1);
                         
                         hist_cursor++;
                         
                         strcpy(buf, history[hist_cursor]);
                         len = strlen(buf);
                         pos = len; 
 
                         printf("\r\033[Kuser> %s\033[K", buf); 
                     }
                 } 
                 else if (next2 == 'B') {
                     if (hist_cursor > 0) {
                         hist_cursor--;
 
                         strcpy(buf, history[hist_cursor]);
                         len = strlen(buf);
                         pos = len;
 
                         printf("\r\033[Kuser> %s\033[K", buf);
                     }
                 }
                 else if (next2 == 'D') {
                     if (pos > 0) {
                         pos--;
                         printf("\033[D");
                     }
                 } 
                 else if (next2 == 'C') {
                     if (pos < len) {
                         pos++;
                         printf("\033[C");
                     }
                 }
             }
             continue;
         }
 
         if (c == KEY_BACKSPACE || c == '\b') {
             if (pos > 0) {
                 for (int j = pos - 1; j < len - 1; j++) {
                     buf[j] = buf[j+1];
                 }
                 pos--;
                 len--;
                 buf[len] = '\0';
                 
                 printf("\033[D");
                 for (int j = pos; j < len; j++) putchar(buf[j]);
                 putchar(' ');
                 for (int j = 0; j <= (len - pos); j++) printf("\033[D");
             }
             continue;
         }
 
         if (c == KEY_ENTER || c == '\r') {
             if (len > 0) {
                 history_shift(buf);
             }
             break;
         }
 
         if (c >= 32 && c <= 126) {
             if (len < max_len - 1) {
                 for (int j = len; j > pos; j--) {
                     buf[j] = buf[j-1];
                 }
                 buf[pos] = c;
                 len++;
                 pos++;
                 
                 putchar(c);
                 for (int j = pos; j < len; j++) putchar(buf[j]);
                 for (int j = 0; j < (len - pos); j++) printf("\033[D");
             }
         }
     }
 
     buf[len] = '\0';
     putchar('\n');
     return len;
 }