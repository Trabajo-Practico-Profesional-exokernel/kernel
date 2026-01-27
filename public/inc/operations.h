#ifndef OPERATIONS_H
#define OPERATIONS_H

typedef enum {
    OP_NOOP = 0,
    OP_EXIT,
    OP_EXEC,
    OP_WAIT,
    OP_YIELD,
    OP_GETPID,
    OP_UPTIME,
    OP_SBRK,
    OP_PUTCHAR,
    OP_GETCHAR,
    OP_OPEN,
    OP_CLOSE,
    OP_READ,
    OP_WRITE,
    OP_LSEEK,
    OP_FSTAT,
    OP_DUP,
    OP_PIPE,
    OP_MKDIR,
    OP_RMDIR,
    OP_CHDIR,
    OP_PWD,
    OP_LS,
    OP_MKNOD,
    OP_LINK,
    OP_UNLINK,
    OP_CHOWN,
    OP_CHMOD,
    OP_DISK_READ,
    OP_DISK_WRITE,
    OP_REG_HANDLER,
    OP_HANDLER_RET
} SyscallOp;

typedef enum {
    FS_OP_PING = 0,
    FS_OP_OPEN,
    FS_OP_CLOSE,
    FS_OP_READ,
    FS_OP_WRITE,
    FS_OP_LSEEK,
    FS_OP_FSTAT,
    FS_OP_DUP,
    FS_OP_PIPE,
    FS_OP_MKDIR,
    FS_OP_RMDIR,
    FS_OP_CHDIR,
    FS_OP_PWD,
    FS_OP_LS,
    FS_OP_MKNOD,
    FS_OP_LINK,
    FS_OP_UNLINK,
    FS_OP_CHOWN,
    FS_OP_CHMOD
} FilesystemOp;

typedef enum {
    PIPE_OP_PING = 0,
    PIPE_OP_READ,
    PIPE_OP_WRITE,
    PIPE_OP_CLOSE,
    PIPE_OP_FSTAT,
    PIPE_OP_DUP
} PipeOp;

typedef enum {
    CONSOLE_OP_PING = 0,
    CONSOLE_OP_PUTCHAR,
    CONSOLE_OP_GETCHAR,
    CONSOLE_OP_READ,
    CONSOLE_OP_WRITE,
    CONSOLE_OP_OPEN,
    CONSOLE_OP_CLOSE
} ConsoleOp;

typedef enum {
    NONE = 0,
    KERNEL,
    COORD,
    FILESYSTEM,
    PIPE,
    CONSOLE,
    SHELL,
    SERVER_COUNT
} Server;


#endif
