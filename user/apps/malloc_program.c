#include "lib.h"
#include "string.h"
#include "stdlib.h"

void main() {
    printf("MALLOC PROGRAM START\n");

    char * allocated = malloc(4095); // Alloc 1 new page
    char * allocated2 = malloc(1000); // Alloc 1 new page
    char * allocated3 = malloc(1000); // Should not alloc new page.

    malloc_stats();
    
    printf("RECV POINTER %p \n", allocated);

    char * inp = "SOME STRING ";

    printf("COPY '%s' to start of allocated 2 times\n", inp);
    int inp_len = strlen(inp);
    
    strncpy(allocated, inp, inp_len +1);
    strncpy(allocated + inp_len, inp, inp_len +1);

    printf("Allocated value at start %s \n", allocated);

    printf("Now have a page fault?! NO PAGE FAULT SINCE VIRTUAL SBRK NEXT PAGE WAS ALLOCATED!\n");
    strncpy(allocated + 4095, inp, inp_len +1);
    printf("Multi page content ... '%s' \n", allocated+4095);

    // printf("Now DO Actually have a page fault?! End of last page/sbrk end\n");
    // strncpy(allocated2 + 4095, inp, inp_len +1);
    
}