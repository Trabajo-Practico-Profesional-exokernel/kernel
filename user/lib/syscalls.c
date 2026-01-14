#include "inc/syscalls.h"
#include "syscalls.h" // Def of syscalls implemented here.
#include "lib.h" // For printf and syscall func
#include "arch/communication.h"
#include "inc/filesystem.h"
#include "std/string.h"
int exec(int prog_ind, char ** args){
    // convert args pointer to int
    return syscall(SYS_EXEC, prog_ind, (int)(args), 0);
}


int sys_try_send_msg(int proc_pid, char *msg, int type_msg) {
    return syscall(SYS_TRY_SEND_MSG, proc_pid, (int)(msg), type_msg);
}

int sys_try_recv_msg(char *msg) {
    return syscall(SYS_TRY_RECV_MSG, (int)(msg), 0, 0);
}

void sendchar(int proc_pid, char ch) {
    debug_printf("sending char\n");
    syscall(SYS_SENDCHAR, proc_pid, ch, 0);
}

void sendbyte(int proc_pid, char ch) {
    syscall(SYS_SEND_BYTE, proc_pid, ch, 0);
}

char recvbyte() {
    return syscall(SYS_RECV_BYTE, 0, 0, 0);
}

int sys_recv_msg(struct Message *msg) {
    return syscall(SYS_RECV_MSG, (int)msg, sizeof(struct Message), 0);
}

void recvchar(int proc_pid, char ch) {
    debug_printf("receiving char\n");
    syscall(SYS_RECVCHAR, proc_pid, ch, 0);
}

void putchar(char ch) {
    syscall(SYS_PUTCHAR, ch, 0, 0);
}

int getchar(void) {
    return syscall(SYS_GETCHAR, 0, 0, 0);
}


int wait(int pid){
    return syscall(SYS_WAIT, pid, 0, 0);
}

void sys_yield(){
    syscall(SYS_YIELD, 0, 0, 0);
}

__attribute__((noreturn)) void exit(int ret_code) {
    syscall(SYS_EXIT, ret_code, 0, 0);
    // SHOULD NEVER HAPPEN... just to make compiler shutup
    printf("SHOULD NOT REACH HERE! AFTER EXIT\n");
    for(;;){

    }
}

void * sbrk(const int count_pages){
    return (void *) syscall(SYS_SBRK, count_pages, 0, 0);
}



///
/// FILESYSTEM
///

int register_fs_handler(struct FilesystemEventsHandler* handler){
    return syscall(SYS_FS_REG_HANDLER, (int) handler, 0 , 0);
}

void sys_fs_ret(int ret_code){
    syscall(SYS_FS_RET, ret_code, 0 , 0);
}


int sys_rm(char* filepath){
    int res = sys_try_send_msg(99, filepath, FS_TYPE_UNLINK);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    if (res >= 0) {
        return atoi(respuesta.content);
    }
    return -1;
}

int sys_stat(char* filepath){
    int res = sys_try_send_msg(99, filepath, FS_TYPE_FSTAT);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    
    if (res >= 0 && respuesta.content_size > sizeof(int)) {
        return 0;
    }
    
    return -1;
}

int getpid(void) {
    return syscall(SYS_GETPID, 0, 0, 0);
}

int uptime(void) {
    return syscall(SYS_UPTIME, 0, 0, 0);
}

int open(const char *path, int mode) {
    disable_debug_print();
    int res = sys_try_send_msg(99, (void*)path, FS_TYPE_OPEN);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    debug_printf("resultado receive: %d", res);
    if (res >= 0) {
        debug_printf("fd obtenido de lado de proceso: %s\n", respuesta.content);
        debug_printf("fd obtenido de lado de proceso en decimal: %d\n", atoi(respuesta.content));
        // El FS devuelve el FD como un int
        return atoi(respuesta.content);
    }
    return -1;
}

int close(int fd) {
    char fd_str[16]; 
    int_to_string(fd, fd_str);
    int res = sys_try_send_msg(99, fd_str, FS_TYPE_CLOSE);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    if (res >= 0) {
        return atoi(respuesta.content);
    }
    return -1;
}

int fstat(int fd, struct stat *st) {
    return syscall(SYS_FSTAT, fd, (int)st, 0);
}

int mknod(const char *path, short major, short minor) {
    // El FS ignora major/minor en esta implementación
    int res = sys_try_send_msg(99, (void*)path, FS_TYPE_MKNOD);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    if (res >= 0) {
        return atoi(respuesta.content);
    }
    return -1;
}

int unlink(const char *path) {
    int res = sys_try_send_msg(99, (void*)path, FS_TYPE_UNLINK);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    if (res >= 0) {
        return atoi(respuesta.content);
    }
    return -1;
}

int link(const char *old_path, const char *new_path) {
    char buf[256]; // Buffer para concatenar paths
    int len_old = 0;
    while(old_path[len_old] != '\0') len_old++;
    
    int i = 0;
    // Copiar old_path
    for(i=0; i < len_old; i++) buf[i] = old_path[i];
    buf[i++] = ' ';
    
    // Copiar new_path justo después del terminador nulo de old_path
    int j = 0;
    while(new_path[j] != '\0') {
        buf[i++] = new_path[j++];
    }
    buf[i] = '\0';

    int res = sys_try_send_msg(99, buf, FS_TYPE_LINK);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    if (res >= 0) {
        return atoi(respuesta.content);
    }
    return -1;
}


int mkdir(const char *path) {
    int res = sys_try_send_msg(99, (void*)path, FS_TYPE_MKDIR);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    if (res >= 0) {
        return atoi(respuesta.content);
    }
    return -1;
}

int rmdir(const char *path) {
    int res = sys_try_send_msg(99, (void*)path, FS_TYPE_RMDIR);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    if (res >= 0) {
        return atoi(respuesta.content);
    }
    return -1;
}

int chdir(const char *path) {
    int res = sys_try_send_msg(99, (void*)path, FS_TYPE_CHDIR);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    if (res >= 0) {
        return atoi(respuesta.content);
    }
    return -1;
}

int ls(char *path) {
    // Se envía el path aunque la implementación actual de shell_ls use el CWD del proceso
    int res = sys_try_send_msg(99, path, FS_TYPE_LS);
    if (res == 0) {
        return -1;
    }
    struct Message respuesta;
    // Esperamos el ACK del filesystem tras imprimir en consola
    res = sys_recv_msg(&respuesta);
    if (res >= 0) {
        return atoi(respuesta.content);
    }
    return -1;
}

int read_fs(int fd, char *buf, int size) {
    char msg_buffer[MSG_SIZE_MAX];
    char temp[16];

    // Construir string "fd size"
    int_to_string(fd, temp);
    strcpy(msg_buffer, temp);
    strcat(msg_buffer, " ");
    
    int_to_string(size, temp);
    strcat(msg_buffer, temp);

    int res = sys_try_send_msg(99, msg_buffer, FS_TYPE_READ);
    if (res == 0) {
        return -1;
    }

    struct Message respuesta;
    res = sys_recv_msg(&respuesta);
    
    if (res < 0) {
        return -1;
    }

    int bytes_read = respuesta.content_size;
    
    for (int i = 0; i < bytes_read && i < size; i++) {
        buf[i] = respuesta.content[i];
    }

    return bytes_read;
}

int write_fs(int fd, char *content, int len) {
    char msg_buffer[MSG_SIZE_MAX];
    char temp[16];
    
    // Construir header "fd len "
    int_to_string(fd, temp);
    strcpy(msg_buffer, temp);
    strcat(msg_buffer, " ");
    
    int_to_string(len, temp);
    strcat(msg_buffer, temp);
    strcat(msg_buffer, " ");

    // Copiar contenido a continuación del header
    int header_len = strlen(msg_buffer);
    int max_data = MSG_SIZE_MAX - header_len;
    int to_write = (len > max_data) ? max_data : len;

    for (int i = 0; i < to_write; i++) {
        msg_buffer[header_len + i] = content[i];
    }

    int res = sys_try_send_msg(99, msg_buffer, FS_TYPE_WRITE);
    if (res == 0) {
        return -1;
    }

    struct Message respuesta;
    res = sys_recv_msg(&respuesta);

    if (res >= 0) {
        return atoi(respuesta.content);
    }

    return -1;
}


int read(int fd, char *buf, int size) {
    return syscall(SYS_READ, fd, (int)buf, size);
}

int write(int fd, char *content, int len) {
    return syscall(SYS_WRITE, fd, (int)content, len);
}

int lseek(int fd, int offset, int whence) {
    char msg_buffer[MSG_SIZE_MAX];
    char temp[16];

    // Construir string "fd offset"
    int_to_string(fd, temp);
    strcpy(msg_buffer, temp);
    strcat(msg_buffer, " ");

    int_to_string(offset, temp);
    strcat(msg_buffer, temp);

    int res = sys_try_send_msg(99, msg_buffer, FS_TYPE_LSEEK);
    if (res == 0) {
        return -1;
    }

    struct Message respuesta;
    res = sys_recv_msg(&respuesta);

    if (res >= 0) {
        return atoi(respuesta.content);
    }

    return -1;
}

int getcwd(char *buf, int size) {
    int res = sys_try_send_msg(99, "pwd", FS_TYPE_PWD);

    if (res == 0) {
        return -1;
    }

    struct Message respuesta;
    res = sys_recv_msg(&respuesta);

    if (res >= 0) {
        int len = respuesta.content_size;
        if (len > size) len = size;
        
        for(int i = 0; i < len; i++) {
            buf[i] = respuesta.content[i];
        }

        if (size > 0) buf[size - 1] = '\0';
        
        return 0;
    }

    return -1;
}

int pipe(int fds[2]){
    return syscall(SYS_PIPE, (int)fds, 0, 0);
}

int dup(int prev_fd){
    return syscall(SYS_DUP, prev_fd, 0, 0);
}
