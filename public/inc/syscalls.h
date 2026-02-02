#ifndef PUBLIC_INC_SYSCALLS
#define PUBLIC_INC_SYSCALLS

#define SYS_PUTCHAR 0
#define SYS_GETCHAR 1

#define SYS_EXEC 2
#define SYS_EXIT 3
#define SYS_WAIT 4
#define SYS_YIELD 5
#define SYS_GETPID 6
#define SYS_UPTIME 7
#define SYS_SBRK 8

#define SYS_DISK_READ 9
#define SYS_DISK_WRITE 10

#define SYS_TRY_SEND_CONTENT 11
#define SYS_TRY_RECV_CONTENT 12
#define SYS_RECV_CONTENT 13
#define SYS_COORDPID 14
#define SYS_ALIVE 15
#define SYS_VIRTUAL_COPY 16

#define SYS_KILL 17
#define SYS_FORK 18
#define SYS_SLEEP 19

#define DEF_ERR_CODE -1



#endif
