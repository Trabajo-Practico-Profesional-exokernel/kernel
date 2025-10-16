#ifndef INC_TRAP_ENTRY
#define INC_TRAP_ENTRY

//__attribute__((naked))
//__attribute__((aligned(4)))
//void trap_entry(void);

//void handle_trap(FullTrapFrame *f);

// Para riscv no hace falta el resto de metodos almenos. El resto son internos/solo usados 
// por trap.c del arch
void init_trap(void);
#endif /* !*/
