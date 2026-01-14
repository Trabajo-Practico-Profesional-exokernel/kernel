#ifndef PUBLIC_INC_SYSCALLS
#define PUBLIC_INC_SYSCALLS

#define SYS_PUTCHAR 0
#define SYS_GETCHAR 1

#define SYS_EXEC 2

#define SYS_EXIT 3
#define SYS_WAIT 4

#define SYS_YIELD 5

#define SYS_TRY_SEND_MSG 6
#define SYS_TRY_RECV_MSG 7

#define SYS_SENDCHAR 8
#define SYS_RECVCHAR 9

#define SYS_SEND_BYTE 10
#define SYS_RECV_BYTE 11

#define SYS_FS_REG_HANDLER 15
#define SYS_FS_RET 16


#define SYS_DISK_READ 17
#define SYS_DISK_WRITE 18

#define SYS_GETPID 19
#define SYS_UPTIME 20

#define SYS_SEND_MSG 21
#define SYS_RECV_MSG 22

#define SYS_READ 23
#define SYS_WRITE 24

#define SYS_SBRK 31

#define SYS_FSTAT 32
#define SYS_PIPE 33
#define SYS_DUP 34
#define SYS_CHOWN 35
#define SYS_CHMOD 36

#define DEF_ERR_CODE -1



#endif
