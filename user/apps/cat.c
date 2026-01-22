#include "disk_syscalls.h"
#include "lib.h"


void
main()
{
	char wbuf[64] = "Hola";
	char rbuf[64] = { 0 };

	int w = disk_write(wbuf, 0, 64);
	printf("wstatus: %d\n", w);

	int r = disk_read(0, rbuf, 64);
	printf("rstatus: %d\n", r);
	printf("rbuf: %s\n", rbuf);

}

