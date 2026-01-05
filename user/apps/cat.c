#include "disk_syscalls.h"
#include "lib.h"


void
main()
{
	char buf[64];
	int r = disk_read(64, buf, 64);
	printf("status: %d\n", r);

	for (int i = 0; i < 64; i++) {
		printf("%c ", buf[i]);
	}
}

