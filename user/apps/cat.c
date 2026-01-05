#include "disk_syscalls.h"
#include "lib.h"


void
main()
{
	char buf[64];
	sys_disk_read(0, buf, 64);
	printf("%s\n", buf);
}

