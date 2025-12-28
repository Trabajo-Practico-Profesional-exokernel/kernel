#include "lib.h"
#include "syscalls.h"
#include "std/string.h"

#define SLEEP_TIME 300000000

void main() {
    int count = 0;
    while (count < 50){
        printf("PROC A %d! My PID is [%u], UPTIME: [%u]\n", count, getpid(), uptime());
        sleep(SLEEP_TIME);
        count++;
    }
    /*
    // revisar este envio de mensaje ya que se enviaron bytes adicionales
    printf("PROC A %d! My PID is [%u], UPTIME: [%u]\n", count, getpid(), uptime());
    char *content_send = "holuuuu como te vaa";
        printf("SENDING MSG [%s] to ME WITH PID [%u] AND LEN CONTENT [%u]\n", content_send, getpid(), strlen(content_send));
        int send_success = try_send_msg(1, content_send, strlen(content_send));

        if (!send_success) {
            printf("SOMETHING WENT WRONG DURING THE MSG SENDING\n");
            return;
        }
        printf("MSG SENDED\n");
    while (count < 50){
        printf("PROC A %d! My PID is [%u], UPTIME: [%u]\n", count, getpid(), uptime());
        sleep(SLEEP_TIME);
        count++;
    }*/
}