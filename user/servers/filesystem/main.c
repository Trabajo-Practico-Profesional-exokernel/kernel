#include "lib.h"
#include "inc/filesystem.h"
#include "inc/syscalls.h"
#include "arch/communication.h"
#include "std/string.h"
#include "disk_syscalls.h"
#include "filesystem.h"

void send_ok_response(int pid_target) {
    char *response = "OK";
    sys_try_send_msg(pid_target, response, 2); 
}


void handle_open(struct Message *msg) {
    printf("[FS] OPEN request from PID %d: path='%s'\n", msg->sender_pid, msg->content);
    // TODO: Lógica real de open
    send_ok_response(msg->sender_pid);
}

void handle_close(struct Message *msg) {
    printf("[FS] CLOSE request from PID %d\n", msg->sender_pid);
    // TODO: Lógica real de close
    send_ok_response(msg->sender_pid);
}

void handle_mkdir(struct Message *msg) {
    printf("[FS] MKDIR request from PID %d: path='%s'\n", msg->sender_pid, msg->content);
    // TODO: Lógica real de mkdir
    send_ok_response(msg->sender_pid);
}

void handle_rm(struct Message *msg) {
    printf("[FS] REMOVE request from PID %d: path='%s'\n", msg->sender_pid, msg->content);
    // TODO: Lógica real de remove
    send_ok_response(msg->sender_pid);
}

void handle_fstat(struct Message *msg) {
    printf("[FS] FSTAT request from PID %d\n", msg->sender_pid);
    // TODO: Lógica real de fstat (debería devolver struct stat, no solo OK)
    send_ok_response(msg->sender_pid);
}

void handle_link(struct Message *msg) {
    printf("[FS] LINK request from PID %d\n", msg->sender_pid);
    // TODO: Lógica real de link
    send_ok_response(msg->sender_pid);
}

void handle_mknod(struct Message *msg) {
    printf("[FS] MKNOD request from PID %d\n", msg->sender_pid);
    // TODO: Lógica real de mknod
    send_ok_response(msg->sender_pid);
}

void handle_chdir(struct Message *msg) {
    printf("[FS] CHDIR request from PID %d\n", msg->sender_pid);
    // TODO: Lógica real de chdir
    send_ok_response(msg->sender_pid);
}

void test_disk_write() {
    char buffer[512]; // Tamaño obligatorio por check_valid_size_write
    
    // 1. Limpiar buffer
    memset(buffer, 0, 512);
    
    // 2. Escribir "hola" al inicio
    char *msg = "hola";
    memcpy(buffer, msg, 5); // Copiar 'h','o','l','a','\0'

    // 3. Llamar a la función existente: buffer, sector 0, tamaño 512
    printf("[FS] Intentando escribir 'hola' en sector 0...\n");
    int res = disk_write(buffer, 0, 512);

    if (res < 0) {
        printf("[FS] Error escribiendo en disco: %d\n", res);
    } else {
        printf("[FS] Escritura exitosa en sector 0.\n");
    }
}
void dispatch_request(struct Message *msg) {
    switch (msg->type) {
        case FS_TYPE_OPEN:   handle_open(msg); break;
        case FS_TYPE_CLOSE:  handle_close(msg); break;
        case FS_TYPE_MKDIR:  handle_mkdir(msg); break;
        case FS_TYPE_UNLINK: handle_rm(msg); break;
        case FS_TYPE_FSTAT:  handle_fstat(msg); break;
        case FS_TYPE_LINK:   handle_link(msg); break;
        case FS_TYPE_MKNOD:  handle_mknod(msg); break;
        case FS_TYPE_CHDIR:  handle_chdir(msg); break;
        default:
            printf("[FS] Unknown msg type %d from PID %d\n", msg->type, msg->sender_pid);
            send_ok_response(msg->sender_pid); 
            break;
    }
}

void server_listen(){
    struct Message msg;
    while(1) {
        printf("WAITING FOR NEW MSG...\n");
        int res = sys_recv_msg(&msg); 
        
        if (res == 0) { 
            dispatch_request(&msg);
        } else {
            printf("SOMETHING WENT WRONG DURING THE MSG RECEIVING\n");
            return;
        }
    }
}

void init_fs(){
    printf("[FS] Service Initialized. Waiting for messages...\n");
    fs_init();
}

void main(){
    init_fs();
    server_listen();
}