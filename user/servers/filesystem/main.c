/* main.c */
#include "lib.h"
#include "inc/filesystem.h"
#include "inc/syscalls.h"
#include "arch/communication.h"
#include "std/string.h"
#include "disk_syscalls.h"

#include "fs.h"
#include "util.h"
#include "fsUtil.h"
#include "block.h"
#include "common.h"
#include "inc/filesystem.h"

extern char current_path[PROCS_MAX][MAX_PATH_NAME];

void send_int_response(int pid_target, int value) {
    sys_try_send_msg(pid_target, (char *)&value, 1);
}

void send_data_response(int pid_target, void *data, int size) {
    sys_try_send_msg(pid_target, (char *)data, 1);
}

void handle_open(struct Message *msg) {
    int fd = fs_open((char *)msg->content, FS_O_RDWR, msg->sender_pid);
    char fd_str[16]; 
    int_to_string(fd, fd_str);
    send_data_response(msg->sender_pid, fd_str, strlen(fd_str) + 1);
}

void handle_close(struct Message *msg) {
    int fd = atoi(msg->content);
    int res = fs_close(fd, msg->sender_pid);
    char res_str[16];
    int_to_string(res, res_str);
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_read(struct Message *msg) {
    char *args = (char *)msg->content;
    
    // Parsear fd
    char *size_str = strchr(args, ' ');
    if (size_str) {
        *size_str = '\0'; // Terminar string del fd
        size_str++;       // Avanzar al size
    } else {
        send_data_response(msg->sender_pid, NULL, 0);
        return;
    }

    int fd = atoi(args);
    int count = atoi(size_str);

    char buffer[1024];
    int to_read = (count > sizeof(buffer)) ? sizeof(buffer) : count;
    
    int bytes_read = fs_read(fd, buffer, to_read, msg->sender_pid);

    if (bytes_read < 0) {
        send_data_response(msg->sender_pid, NULL, 0);
    } else {
        send_data_response(msg->sender_pid, buffer, bytes_read);
    }
}

void handle_write(struct Message *msg) {
    char *args = (char *)msg->content;

    // Parsear fd
    char *len_str = strchr(args, ' ');
    if (!len_str) return;
    *len_str = '\0';
    len_str++;

    // Parsear len
    char *data_ptr = strchr(len_str, ' ');
    if (!data_ptr) return;
    *data_ptr = '\0';
    data_ptr++;

    int fd = atoi(args);
    int count = atoi(len_str);

    // Validar size real vs header (seguridad)
    // data_ptr apunta al inicio de los datos en el mensaje
    // Calculamos cuanto espacio queda en el mensaje
    int header_size = (data_ptr - args);
    int valid_len = msg->content_size - header_size;
    
    if (count > valid_len) count = valid_len;

    int bytes_written = fs_write(fd, data_ptr, count, msg->sender_pid);
    
    char res_str[16];
    int_to_string(bytes_written, res_str);
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_lseek(struct Message *msg) {
    char *args = (char *)msg->content;

    // Parsear fd
    char *offset_str = strchr(args, ' ');
    if (offset_str) {
        *offset_str = '\0';
        offset_str++;
    }

    int fd = atoi(args);
    int offset = atoi(offset_str);

    int res = fs_lseek(fd, offset, msg->sender_pid);
    
    char res_str[16];
    int_to_string(res, res_str);
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_mkdir(struct Message *msg) {
    int result = fs_mkdir((char *)msg->content, msg->sender_pid);
    if (result == 0) {
        fs_sync_current_dir(msg->sender_pid); 
    }
    char res_str[16];
    int_to_string(result, res_str);
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_rmdir(struct Message *msg) {
    int result = fs_rmdir((char *)msg->content, msg->sender_pid);
    if (result == 0) {
        fs_sync_current_dir(msg->sender_pid); 
    }
    char res_str[16];
    int_to_string(result, res_str);
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_rm(struct Message *msg) {
    int result = fs_unlink((char *)msg->content, msg->sender_pid);
    char res_str[16];
    int_to_string(result, res_str);
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_fstat(struct Message *msg) {
    fileStat stat_buf;
    int res = fs_stat((char *)msg->content, &stat_buf, msg->sender_pid);

    printf("Inode: %d | Type: %c | Links: %d | Size: %d | Blocks: %d \nowner perms: %x | group perms: %x | other perms: %x\n", 
        stat_buf.inodeNo, 
        (stat_buf.type == DIRECTORY) ? 'D' : 'F', 
        (int)stat_buf.links, 
        stat_buf.size, 
        stat_buf.numBlocks,
        stat_buf.owner_perms,
        stat_buf.group_perms,
        stat_buf.other_perms);
        
    if (res == 0) {
        send_int_response(msg->sender_pid, 0);
    } else {
        send_int_response(msg->sender_pid, -1);
    }
}

void handle_link(struct Message *msg) {
    char *old = (char *)msg->content;
    char *new = strchr(old, ' ');

    if (new != NULL) {
        *new = '\0';
        new++;
    } else {
        char res_str[16];
        int_to_string(-1, res_str);
        send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
        return;
    }

    int res = fs_link(old, new, msg->sender_pid);
    char res_str[16];
    int_to_string(res, res_str);
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_chdir(struct Message *msg) {
    int res = fs_cd((char *)msg->content, msg->sender_pid);
    char res_str[16];
    int_to_string(res, res_str);
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_mknod(struct Message *msg) {
    int fd = fs_open((char *)msg->content, FS_O_RDWR, msg->sender_pid);
    char res_str[16];
    if (fd >= 0) {
        fs_close(fd, msg->sender_pid);
        int_to_string(0, res_str);
    } else {
        int_to_string(-1, res_str);
    }
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_shell_ls(struct Message *msg){
    shell_ls(msg->sender_pid);
    char res_str[16];
    int_to_string(0, res_str);
    send_data_response(msg->sender_pid, res_str, strlen(res_str) + 1);
}

void handle_pwd(struct Message *msg) {
    char *path = current_path[msg->sender_pid];
    send_data_response(msg->sender_pid, path, strlen(path) + 1);
}

void handle_chown(struct Message *msg) {
    //cambiar este handleo luego con algun refactor
    char *args[3] = {NULL, NULL, NULL};
    int n = parse_3_args(msg->content, args);
    debug_printf("n [%d]", n);
    if (n<3){
        send_int_response(msg->sender_pid, -1);
        return;
    }

    int res = fs_chown(msg->sender_pid, args[0], atoi(args[1]), atoi(args[2]));
        
    if (res == 0) {
        send_int_response(msg->sender_pid, 0);
    } else {
        send_int_response(msg->sender_pid, -1);
    }
}

void handle_chmod(struct Message *msg) {
    char *path;
    int mode;
    if (parse_str_int(msg->content, &path, &mode) == 0) {
        
        fs_chmod(msg->sender_pid, path, mode);
        send_int_response(msg->sender_pid, 0);
        return;
    }

    printf("Uso: chmod <path> <mode>\n");
    send_int_response(msg->sender_pid, -1);
}


#define MAX_HANDLERS (sizeof(dispatch_table) / sizeof(dispatch_table[0]))

typedef void (*fs_handler_t)(struct Message *);

static const fs_handler_t dispatch_table[] = {
    [FS_TYPE_OPEN]   =   handle_open,
    [FS_TYPE_CLOSE]  =   handle_close,
    [FS_TYPE_READ]   =   handle_read,
    [FS_TYPE_WRITE]  =   handle_write,
    [FS_TYPE_MKDIR]  =   handle_mkdir,
    [FS_TYPE_RMDIR]  =   handle_rmdir,
    [FS_TYPE_UNLINK] =   handle_rm,
    [FS_TYPE_FSTAT]  =   handle_fstat,
    [FS_TYPE_LINK]   =   handle_link,
    [FS_TYPE_CHDIR]  =   handle_chdir,
    [FS_TYPE_LSEEK]  =   handle_lseek,
    [FS_TYPE_MKNOD]  =   handle_mknod,
    [FS_TYPE_LS]     =   handle_shell_ls,
    [FS_TYPE_PWD]    =   handle_pwd,
    [FS_TYPE_CHOWN]  =   handle_chown,
    [FS_TYPE_CHMOD]  =   handle_chmod,
};

void dispatch_request(struct Message *msg) {
    if (msg->type >= 0 && msg->type < MAX_HANDLERS && dispatch_table[msg->type]) {
        dispatch_table[msg->type](msg);
    } else {
        debug_printf("[FS] Unknown msg type %d\n", msg->type);
        send_int_response(msg->sender_pid, -1);
    }
}

void server_listen() {
    struct Message msg;
	debug_printf("filesystem server is running");

    while (1) {
        int res = sys_recv_msg(&msg);
        if (res == 0) {
            debug_printf("mensaje recibido con contenido: [%d], [%s]", msg.type, msg.content);
            dispatch_request(&msg);
        }
    }
}

void main() {
    disable_debug_print();
    debug_printf("FILESYSTEM!");
    fs_init();
	shell_ls(2);
    //server_listen();
}
