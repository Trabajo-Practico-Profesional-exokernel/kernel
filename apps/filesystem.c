#include "lib.h"

#define SLEEP_TIME 300000000

void main() {
    while (1){
        printf("esperando mensaje!");
        char msg = recv_msg(); 
        printf("Recibido en proceso: %c \n", msg);
        sleep(SLEEP_TIME);
    }
    

}