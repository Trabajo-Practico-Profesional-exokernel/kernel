#include "lib.h"

#include "inc/filesystem.h"
#include "disk_syscalls.h"
#include "std/string.h"
#include "syscalls.h"

#define SLEEP_TIME 300000000
#define MSG_SIZE_MAX 64
#define DISK_SECTOR_SIZE 512

char buffer[MSG_SIZE_MAX];
struct FilesystemEventsHandler fs_events_handler;

void handler_on_touch(int msg_len){
    printf("USER FS on touch msg len %d path: ", msg_len);
    printf("'%s'\n", &buffer[0]);
    sys_fs_ret(5);
}

void handler_on_rm(int msg_len){
    printf("USER FS on remove msg len %d path: ", msg_len);
    printf("'%s'\n", &buffer[0]);
    
    sys_fs_ret(5);
}

void handler_on_stat(int msg_len){
    printf("USER FS on stat msg len %d path: ", msg_len);
    printf("'%s'\n", &buffer[0]);
    sys_fs_ret(5);
}

/*
void main() {

    fs_events_handler.buffer = &buffer;
    fs_events_handler.buffer_len = MSG_SIZE_MAX-1; // Just in case have the last byte always 0!
    buffer[MSG_SIZE_MAX -1] = 0; 

    
    fs_events_handler.on_touch = handler_on_touch;
    fs_events_handler.on_stat = handler_on_stat;
    fs_events_handler.on_rm = handler_on_rm;

    char buff_write[DISK_SECTOR_SIZE];
    char* VALUE = "SOME MESSAGE VALUE";
    strcpy(&buff_write[0], VALUE);
    
    printf("=====> USER FS WRITING TO DISK 0 => '%s'\n", VALUE);
    int write_superblock_ret = disk_write(VALUE, 0, DISK_SECTOR_SIZE);

    printf("=====> WRITE RET %d\n", write_superblock_ret);

    char ret[100];

    int read_superblock_ret = disk_read(0, &ret[0], strlen(VALUE));

    printf("USER FS READ disk READ RESULT %d  CONTENT => '%s'\n",read_superblock_ret, &ret[0]);

    int err = register_fs_handler(&fs_events_handler);
    if (err != 0) {
        printf("Failed registering FS handler %d\n", err);
    } else {
        printf("FS finished.. it was mounted down? or was it a mistake?\n");
    }

}*/

void init_fs(){
    printf("init filesystem \n");
}

void server_listen(){
    for(;;){
        printf("WAITING FOR NEW MSG...\n");
        char content[64];
        int pid_sender = recv_msg(&content[0], 64);
        if (pid_sender >= 0){
            printf("MSG RECEIVED WITH CONTENT: [%s]\n", content);

            char *response = "OK";
            try_send_msg(pid_sender, response, strlen(response));

        } else {
            printf("SOMETHING WENT WRONG DURING THE MSG RECEIVING\n");
            return;
        }
    }
}



void main(){
    init_fs();
    server_listen();
}