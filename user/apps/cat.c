#include "disk_syscalls.h"


void
main(int argc, char argv[])
{
	char buf[64];
	sys_disk_read(0, buf, 64)
	printf("%s\n", buf);
	return 0;
}

