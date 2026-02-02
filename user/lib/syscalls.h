#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"
#include "inc/filesystem.h"


int exec(int prog_ind, char ** args);
int wait(int pid);
int sys_kill(int pid);

int sys_fork(void);
int sys_sleep(int time);


void sys_yield(void);
__attribute__((noreturn)) void exit(int ret_code);
int getpid(void);
void * sbrk(const int count_pages);
int uptime(void);
int alive(int pid);
int get_coord_pid(void);

void putchar(char ch);
int getchar(void);
int try_send_content(int proc_pid, char *content, int len_content);
int try_recv_content(char *content, int len_content);
int recv_content(char *content, int len_content);
int virtual_copy(uint32_t pid_src_proc, uint32_t src_addr, uint32_t dst_addr, int len);

int open(const char *path, int mode);
int close(int fd);
int read(int fd, char *buf, int size);
int write(int fd, char *content, int len);
int lseek(int fd, int offset, int whence);
int stat(char* filepath);

int mkdir(const char *path);
int rmdir(const char *path);
int chdir(const char *path);
int getcwd(char *buf, int size);
int ls(const char *path);

int mknod(const char *path, short major, short minor);
int link(const char *old_path, const char *new_path);
int unlink(const char *path);

int chown(const char *pathname, uint32_t owner, uint32_t group);
int chmod(const char *pathname, uint32_t mode);

int pipe(int fds[2]);
int dup(int prev_fd);

#endif
