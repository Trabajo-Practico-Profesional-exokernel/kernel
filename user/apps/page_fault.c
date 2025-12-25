#include "lib.h"

void main() {
    printf("Will do a page fault!\n");
    char * pointer = (char *)0x200000;
    *pointer = 'a';
}