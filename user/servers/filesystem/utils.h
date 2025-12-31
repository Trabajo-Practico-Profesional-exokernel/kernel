#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include "disk_syscalls.h"
#include "filesystem.h"

int fs_get_free_inode(void);

void fs_release_inode(int inode_idx);
#endif
