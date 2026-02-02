#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"

#define SLEEP_TIME 2
char * INPUT_STR_BASE = "SOME STRING ";

void main() {
	int count = 1;
    char * allocated = sbrk(1); // Alloc 1 page

	while(count < 3){
		printf("[FRK] WAIT n%d\n", count);
		count+=1;
		sys_sleep(SLEEP_TIME);
	}


    printf("[FRK] First COPY '%s' to start of srbk page\n", INPUT_STR_BASE);
    int inp_len = strlen(INPUT_STR_BASE);
    strncpy(allocated, INPUT_STR_BASE, inp_len +1);
    printf("[FRK] allocated STR '%s'\n", allocated);

	printf("[FRK] DO FORK NOW! n%d\n", count);
	int ret = sys_fork();

	if(ret < 0){
		printf("Failed fork! %d\n", ret);
		return;
	}

	if(ret == 0){
		printf("[child] Only Child Enters here! count: %d \n", count);
		char* child_str = "NOW IS CHILD!";
	    printf("[child] COPY '%s' to start of srbk page\n", child_str);
	    inp_len = strlen(child_str);
	    strncpy(allocated, child_str, inp_len +1);

		while(count < 50){
			printf("[child] WAIT n%d\n", count);
			count+=1;
			sys_sleep(SLEEP_TIME);
		}

	    printf("[child] VL allocated STR '%s'\n", allocated);

		return;
	}
	
	printf("[parent] Only Parent Enters here! Child has pid %d, count: %d\n", ret, count);
	
	char* parent_str = "NOW IS PARENT!";
    printf("[parent] COPY '%s' to start of srbk page\n", parent_str);
    inp_len = strlen(parent_str);
    strncpy(allocated, parent_str, inp_len +1);

	while(count < 50){
		printf("[parent] WAIT n%d\n", count);
		count+=1;
		sys_sleep(SLEEP_TIME);
	}
    printf("[parent] VL allocated STR '%s'\n", allocated);
}