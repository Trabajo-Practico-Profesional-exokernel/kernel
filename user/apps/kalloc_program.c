#include "lib.h"
#include "std/string.h"

void main() {
    printf("KALLOC PROGRAM START\n");

    char * allocated = sbrk(1); // Alloc 1 page

    printf("RECV POINTER %p \n", allocated);

    char * inp = "SOME STRING ";

    printf("COPY '%s' to start of allocated 2 times\n", inp);
    int inp_len = strlen(inp);
    
    strncpy(allocated, inp, inp_len +1);
    strncpy(allocated + inp_len, inp, inp_len +1);

    printf("Allocated value at start %s \n", allocated);

    printf("Now have a page fault?! \n");
    strncpy(allocated + 4095, inp, inp_len +1);
    
}