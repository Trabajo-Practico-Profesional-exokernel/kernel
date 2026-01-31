#include "tests_hardware.h"
#include "test_common.h"
#include "stdio.h"
#include "arch/mem.h"
#include "string.h"
#include "stdlib.h"
#include "test_utils.h"

// Includes específicos de arquitectura x86 (supuestos)
// Si no tienes un archivo de io.h, las funciones inb/outb se definen abajo inline
#include "arch_inc/mem_constants.h" 

#define TEST_VIRT_ADDR  0x40000000
#define VGA_BUFFER      0xB8000
#define COM1_PORT       0x3F8

// Variables externas del kernel x86
extern char __kernel_base[];
// En x86, la tabla raíz se suele llamar kernel_pdeectory o es el valor de CR3
extern uint32_t kernel_pde[]; 

// -------------------------------------------------------------------------
// HELPERS: Inline Assembly para x86
// -------------------------------------------------------------------------
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t read_cr0(void) {
    uint32_t val;
    __asm__ volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}

static inline uint32_t read_cr3(void) {
    uint32_t val;
    __asm__ volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline uint32_t read_eflags(void) {
    uint32_t val;
    __asm__ volatile("pushf; pop %0" : "=r"(val));
    return val;
}

static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static inline void invlpg(void* m) {
    __asm__ volatile("invlpg (%0)" : : "r"(m) : "memory");
}

// -------------------------------------------------------------------------
// TEST 1: Video Memory Integrity (VGA Text Mode)
// Escribe un carácter en la memoria de video y lo lee de vuelta.
// Equivale a probar MMIO pero en una dirección fija de x86.
// -------------------------------------------------------------------------
void test_vga_integrity(CTest *ctx) {
    volatile uint16_t *vga_mem = (uint16_t*)VGA_BUFFER;
    
    // Guardamos el valor original de la esquina superior izquierda
    uint16_t original = vga_mem[0];
    
    // Escribimos 'X' (0x58) con atributo blanco sobre azul (0x1F)
    uint16_t test_val = 0x1F58; 
    vga_mem[0] = test_val;
    
    // Verificamos si se escribió
    CTEST_ASSERT_EQ(ctx, test_val, vga_mem[0], "VGA Buffer R/W Integrity");
    
    // Restauramos
    vga_mem[0] = original;
}

// -------------------------------------------------------------------------
// TEST 2: Serial Port Loopback (COM1)
// Configura el UART en modo loopback para verificar transmisión/recepción.
// -------------------------------------------------------------------------
void test_serial_loopback(CTest *ctx) {
    // 1. Activar Loopback Mode en Modem Control Register (Offset 4)
    // Bit 4 = Loopback
    uint8_t mcr_old = inb(COM1_PORT + 4);
    outb(COM1_PORT + 4, mcr_old | 0x10); 

    // 2. Escribir un byte de prueba
    uint8_t test_char = 0xA5;
    outb(COM1_PORT, test_char);

    // 3. Leer el byte de vuelta
    // En loopback, lo que escribes en TX vuelve inmediatamente a RX
    uint8_t read_char = inb(COM1_PORT);

    CTEST_ASSERT_EQ(ctx, test_char, read_char, "Serial Port (COM1) Loopback Test");

    // 4. Restaurar MCR
    outb(COM1_PORT + 4, mcr_old);
}

// -------------------------------------------------------------------------
// TEST 3: CPU Time Stamp Counter (TSC)
// Verifica que el contador de ciclos del procesador avanza.
// -------------------------------------------------------------------------
void test_cpu_tsc(CTest *ctx) {
    uint64_t t1 = rdtsc();
    
    // Busy wait
    for(volatile int i = 0; i < 10000; i++); 
    
    uint64_t t2 = rdtsc();

    CTEST_ASSERT_GT(ctx, (int)(t2 - t1), 0, "CPU TSC (rdtsc) increment check");
}

// -------------------------------------------------------------------------
// TEST 4: Control Registers (CR0 & CR3)
// Verifica que la paginación y el modo protegido estén activos.
// -------------------------------------------------------------------------
void test_control_registers(CTest *ctx) {
    // 1. Test CR0 (Protected Mode & Paging)
    uint32_t cr0 = read_cr0();
    
    // Bit 0 = PE (Protection Enable)
    CTEST_ASSERT_TRUE(ctx, cr0 & 0x1, "CR0: Protected Mode Enabled (PE)");
    // Bit 31 = PG (Paging)
    CTEST_ASSERT_TRUE(ctx, cr0 & 0x80000000, "CR0: Paging Enabled (PG)");

    // 2. Test CR3 (Page Directory Base)
    uint32_t cr3 = read_cr3();
    CTEST_ASSERT_NOT_NULL(ctx, (void*)cr3, "CR3 is set (Not NULL)");
    CTEST_ASSERT_EQ(ctx, 0, cr3 & 0xFFF, "CR3 is 4KB Aligned");
}

// -------------------------------------------------------------------------
// TEST 5: Interrupt State (EFLAGS)
// Verifica si las interrupciones están habilitadas (IF bit).
// -------------------------------------------------------------------------
void test_interrupts_state(CTest *ctx) {
    uint32_t flags = read_eflags();
    
    // Bit 9 = IF (Interrupt Flag)
    int if_bit = (flags >> 9) & 1;
    
    CTEST_ASSERT_TRUE(ctx, if_bit, "CPU Interrupts (IF) are Enabled");
}

// -------------------------------------------------------------------------
// TEST 6: IDT (Interrupt Descriptor Table)
// Verifica que la tabla de interrupciones esté cargada y sea válida.
// -------------------------------------------------------------------------
struct idtr_t {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void test_idt_config(CTest *ctx) {
    struct idtr_t idtr;
    __asm__ volatile("sidt %0" : : "m"(idtr));

    CTEST_ASSERT_NOT_NULL(ctx, (void*)idtr.base, "IDT Base is not NULL");
    CTEST_ASSERT_GT(ctx, idtr.limit, 0, "IDT Limit > 0");
    
    // Verificar que apunte a espacio de Kernel (generalmente Higher Half o > 1MB)
    // Asumiremos que el kernel está cargado en 0x100000 o superior.
    CTEST_ASSERT_GE(ctx, idtr.base, 0x100000, "IDT located in Kernel Space");
}

// -------------------------------------------------------------------------
// TEST 7: Mecanismos de Paginación x86
// Mapea una dirección, escribe, hace flush TLB y verifica.
// -------------------------------------------------------------------------
void test_paging_mechanisms_x86(CTest *ctx) {
    // 1. Obtener página física
    paddr_t phys_page = alloc_pages(1);
    CTEST_ASSERT_NOT_NULL(ctx, (void*)phys_page, "Allocated physical page");

    vaddr_t virt_addr = TEST_VIRT_ADDR;
    uint32_t perms = 0x3; // Present | Read/Write (Supervisor)

    // 2. Mapear (Requiere implementación de map_page para x86)
    map_page((uint32_t*)kernel_pde, virt_addr, phys_page, perms);

    // 3. FLUSH TLB (Crítico en x86)
    // En RISC-V usábamos sfence.vma, en x86 usamos invlpg
    invlpg((void*)virt_addr);

    // 4. Traducción Inversa
    paddr_t translated = get_paddr_for((uint32_t*)kernel_pde, virt_addr);
    CTEST_ASSERT_EQ(ctx, phys_page, translated, "MMU Translation Check");

    // 5. Test de Escritura Real
    volatile uint32_t *ptr = (uint32_t*)virt_addr;
    *ptr = 0xDEADBEEF;
    
    CTEST_ASSERT_EQ(ctx, 0xDEADBEEF, *ptr, "Virtual Memory R/W Success");
}

// -------------------------------------------------------------------------
// RUNNER PRINCIPAL x86
// -------------------------------------------------------------------------
int run_hardware_tests(void) {
    CTest suite = init_ctx("X86 HARDWARE");

    test_vga_integrity(&suite);
    test_serial_loopback(&suite);
    test_cpu_tsc(&suite);
    // PAGE FAULT
    //test_control_registers(&suite);
    test_interrupts_state(&suite);
    test_idt_config(&suite);
    // PAGE FAULT
    //test_paging_mechanisms_x86(&suite);
    
    return test_run(&suite);
}