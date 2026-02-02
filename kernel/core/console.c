
#include "arch/console.h"
#include "arch/stdio.h"
#include "string.h"
#include "constants.h"

struct Console console;


void console_init(){
    memset(&console, 0, sizeof(struct Console));
}

int console_push_input(uint8_t c) {

    if (console.count >= CONSOLE_BUFFER_SIZE) {
        return ERROR;
    }

    console.input_buf[console.write_idx] = c;
    console.write_idx = (console.write_idx + 1) % CONSOLE_BUFFER_SIZE;
    console.count++;

    return SUCCESS;
}



int32_t console_read(char *buf, int len){

    if (buf == NULL) {
        return ERROR;
    }

    int bytes_read = 0;

    while (bytes_read < len) {
        if (console.count == 0) {
            break;
        }
        
        buf[bytes_read] = console.input_buf[console.read_idx];
        console.read_idx = (console.read_idx + 1) % CONSOLE_BUFFER_SIZE;
        console.count--;
        bytes_read++;
    }

    return bytes_read;
}



int32_t console_write(char *buf, int size){
    if (buf ==  NULL){
        return -1;
    }

    int bytes_writen = 0;

    while (bytes_writen < size){
        putchar(buf[bytes_writen]);
        bytes_writen ++;
    }

    return bytes_writen;
}


int console_add_waiter(int pid){

    if (console.readers_count == MAX_WAITING_PROCS){
        return ERROR;
    }
    
    console.readers[console.readers_count] = pid;
    console.readers_count ++;
    
}

int console_release_waiter_pid(){
    if (console.readers_count == 0){
        return ERROR;
    }

    int pid = console.readers[0];

    for (int i = 0; i < console.readers_count - 1; i++) {
        console.readers[i] = console.readers[i+1];
    }

    console.readers_count --;

    return pid;
}

