#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"

#define SLEEP_TIME 300000000

void main() {
	int count = 1;
	while(true){
		printf("INFINITE LOOP n%d\n", count);
		count+=1;
		sleep(SLEEP_TIME);
	}
}