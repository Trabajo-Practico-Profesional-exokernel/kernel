#include "inc/common.h"
#include "arch/logging.h"

void printTrap(const struct TrapFrame * tf){
    printf("puntero tf %p\n", tf);
}

void printProc(const struct Proc * proc){
    printf("puntero proc %p\n", proc);
}