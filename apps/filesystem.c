#include "lib.h"

#define SLEEP_TIME 300000000
#define MSG_SIZE_MAX 64

void main() {
    char buffer[MSG_SIZE_MAX];
    int index = 0;
    while (1){
        char msg_char = recvbyte();
        buffer[index] = msg_char;
        index ++;
        if (msg_char == '\0'){
            printf("Mensaje recibido: %s \n", buffer);
            break;
        }

        sleep(SLEEP_TIME);
    }
    

}