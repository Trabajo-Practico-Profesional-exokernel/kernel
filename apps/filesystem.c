#include "lib.h"

#define SLEEP_TIME 300000000
#define MSG_SIZE_MAX 64

char buffer[MSG_SIZE_MAX];

void event_handler(int msg_count){
    
}

void main() {
    int err = register_fs_handler(&buffer, MSG_SIZE_MAX, &event_handler);
    // if err == 0
}