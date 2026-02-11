#include "disk_syscalls.h"
#include "lib.h"


void
main()
{
	char wbuf[512] = "Hola";
	wbuf[509] = 'E';
	wbuf[510] = 'N';
	wbuf[511] = 'D';
	char rbuf[512] = { 0 };

	int w = disk_write(wbuf, 0, 512);
	printf("wstatus: %d\n", w);

	int r = disk_read(0, rbuf, 512);
	printf("rstatus: %d\n", r);

	printf("rbuf: ");
	for (int i = 0; i < 512; i++)
		printf("%c", rbuf[i]);
	printf(" [EOL]\n");
}

