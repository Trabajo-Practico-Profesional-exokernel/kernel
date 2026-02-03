#ifndef OPERATIONS_H
#define OPERATIONS_H

typedef enum {
    OP_NOOP = 0,
    OP_OPEN,
    OP_CLOSE,
    OP_READ,
    OP_WRITE,
    OP_LSEEK,
    OP_STAT,
    OP_DUP,
    OP_PIPE,
    OP_MKDIR,
    OP_RMDIR,
    OP_CHDIR,
    OP_CWD,
    OP_LS,
    OP_MKNOD,
    OP_LINK,
    OP_UNLINK,
    OP_CHOWN,
    OP_CHMOD,
    OP_UPDATE, //no es una syscall pero por el momento lo pongo aca
    OP_GET_FD, //no es una syscall pero por el momento lo pongo aca
    OP_GET_SERVER_FD, //no es una syscall pero por el momento lo pongo aca
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
    PIPE_OP_OPEN,
    PIPE_OP_READ,
    PIPE_OP_WRITE,
    PIPE_OP_CLOSE,
    PIPE_OP_FSTAT,
    PIPE_OP_DUP
} PipeOp;

typedef enum {
    CONSOLE_OP_PING = 0,
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
