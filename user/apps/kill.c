#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"

void main(int argc, char ** argv) {
	if(argc < 2){
		printf("Invalid call for kill! Not enough parameters\n");
	}
	printf("Call for kill! '%s' \n", argv[1]);
	
}