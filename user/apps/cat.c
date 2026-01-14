#include "disk_syscalls.h"
#include "lib.h"


void
main()
{
	printf("CAT 1\n");
	char buf[64];
	printf("CAT 2\n");
//	int w = disk_write("ola", 0, 4);
//	printf("wstatus: %d\n", w);
	int r = disk_read(0, &buf[0], 64);
	printf("CAT 3\n");

	printf("rstatus: %d\n", r);

	for (int i = 0; i < 64; i++) {
		printf("%c ", buf[i]);
	}
}

