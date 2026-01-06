#include "disk_syscalls.h"
#include "lib.h"


void
main()
{
	printf("CAT\n");
	char buf[64];
//	int w = disk_write("ola", 0, 4);
//	printf("wstatus: %d\n", w);
	int r = disk_read(0, &buf[0], 64);

	printf("rstatus: %d\n", r);

	for (int i = 0; i < 64; i++) {
		printf("%c ", buf[i]);
	}
}

