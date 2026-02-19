#include "disk_syscalls.h"
#include "lib.h"


void
main()
{
	char buf[512] = "Hola";
	buf[509] = 'E';
	buf[510] = 'N';
	buf[511] = 'D';

	int w = disk_write(buf, 0, 512);
	printf("wstatus: %d\n", w);

	int r = disk_read(0, buf, 512);
	printf("rstatus: %d\n", r);

	printf("rbuf: ");
	for (int i = 0; i < 512; i++)
		printf("%c", buf[i]);
	printf(" [EOL]\n");
}

