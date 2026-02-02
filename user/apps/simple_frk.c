#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"

#define SLEEP_TIME 30000000

void main() {
	int count = 1;
	while(count < 3){
		printf("FRK WAIT n%d\n", count);
		count+=1;
		sleep(SLEEP_TIME);
	}
	printf("DO FRK NOW! n%d\n", count);

	int ret = sys_fork();

	if(ret < 0){
		printf("Failed fork! %d\n", ret);
		return;
	}

	if(ret == 0){
		printf("Children process!! Enters here! count: %d \n", count);
		count = 5;
		return;
	}
	
	printf("Parent process!! Enters here! Child has pid %d, count: %d\n", ret, count);
	count = 5;
}