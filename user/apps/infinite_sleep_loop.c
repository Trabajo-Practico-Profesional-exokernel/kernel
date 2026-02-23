#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "parsers/strutil.h"

#define SLEEP_TIME 300000000

void main(int argc, char ** argv) {
	char* log = "DEFAULT LOG";
	int sleep_time = SLEEP_TIME;
	if(argc >=2){ // First param is exec name
		log= argv[1];
	}

	if(argc >=3){ 
		// Second param sleep time
		sleep_time = atoi(argv[2]);
	}

	
	int count = 1;
	while(true){
		printf("INFINITE LOOP n%d '%s'\n", count, log);
		count+=1;
		sys_sleep(sleep_time);
	}
}