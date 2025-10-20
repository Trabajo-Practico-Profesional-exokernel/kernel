#include "stdio.h"

//Does nothing for now but its defined!?
void main(void) {
    *((volatile int *) 0x80200000) = 0x1234; // Ilegal. No puede acceder a esta addr
    for (;;);
}