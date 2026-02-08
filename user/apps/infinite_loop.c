#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"

#define SLEEP_TIME 300000000

void main(int argc, char ** argv) {
	char* log = "DEFAULT LOG";
	if(argc >=2){ // First param is exec name
		log= argv[1];
	}
	
	int count = 1;
	while(true){
		printf("INFINITE LOOP n%d '%s'\n", count, log);
		count+=1;
		sleep(SLEEP_TIME);
	}
}