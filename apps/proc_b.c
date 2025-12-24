#include "lib.h"
#include "syscalls.h"
#include "std/string.h"
#define SLEEP_TIME 300000000

void main() {
    printf("PROC B %d!\n");

    char *content_send = "holis como andas";
    printf("SENDING MSG [%s] to ME WITH PID [%u] AND LEN CONTENT [%u]\n", content_send, getpid(), strlen(content_send));
    int send_success = try_send_msg(getpid(), content_send, strlen(content_send));

    if (!send_success) {
        printf("SOMETHING WENT WRONG DURING THE MSG SENDING\n");
        return;
    }
    printf("MSG SENDED\n");
    sleep(SLEEP_TIME);

    char content[64];
    printf("RECEIVING MSG\n");
    int success = try_recv_msg(&content[0], 64);
    if (success){
        printf("MSG RECEIVED WITH CONTENT: [%s]\n", content);
    } else {
        printf("SOMETHING WENT WRONG DURING THE MSG RECEIVING\n");
        return;
    }

}