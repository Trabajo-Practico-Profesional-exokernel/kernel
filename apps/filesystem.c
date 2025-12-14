#include "lib.h"

#include "inc/filesystem.h"

#define SLEEP_TIME 300000000
#define MSG_SIZE_MAX 64

char buffer[MSG_SIZE_MAX];
struct FilesystemEventsHandler fs_events_handler;

void handler_on_touch(int msg_len){
    printf("USER FS on touch msg len %d\n", msg_len);
}

void handler_on_rm(int msg_len){
    printf("USER FS on remove msg len %d\n", msg_len);    
}

void handler_on_stat(int msg_len){
    printf("USER FS on stat len %d\n", msg_len);        
}

void main() {

    fs_events_handler.buffer = &buffer;
    fs_events_handler.buffer_len = MSG_SIZE_MAX;
    
    fs_events_handler.on_touch = handler_on_touch;
    fs_events_handler.on_stat = handler_on_stat;
    fs_events_handler.on_rm = handler_on_rm;
    
    int err = register_fs_handler(&fs_events_handler);
    if (err != 0) {
        printf("Failed registering FS handler %d\n", err);
    } else {
        printf("FS finished.. it was mounted down? or was it a mistake?\n");
    }
}