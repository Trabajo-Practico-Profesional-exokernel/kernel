#ifndef PUBLIC_INC_SYSCALLS
#define PUBLIC_INC_SYSCALLS

#define SYS_PUTCHAR 0
#define SYS_GETCHAR 1

#define SYS_EXEC 2

#define SYS_EXIT 3
#define SYS_WAIT 4

#define SYS_YIELD 5


#define SYS_SEND_MSG 6
#define SYS_RECV_MSG 7

#define SYS_SENDCHAR 8
#define SYS_RECVCHAR 9

#define SYS_SEND_BYTE 10
#define SYS_RECV_BYTE 11

#define SYS_FS_TOUCH 12
#define SYS_FS_RM 13
#define SYS_FS_STAT 14

#define SYS_FS_REG_HANDLER 15
#define SYS_FS_RET 16


#define SYS_DISK_READ 17
#define SYS_DISK_WRITE 18

#define DEF_ERR_CODE -1



#endif
