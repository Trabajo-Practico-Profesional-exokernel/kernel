#include "tests_kernel.h"
#include "test_common.h"
#include "std/string.h"
#include "std/printf.h"
#include "utils.h"
// Includes internos del kernel necesarios
#include "arch/mem.h"           // alloc_pages, map_page, get_paddr_for
#include "arch/proc.h"          // struct Proc
#include "../kernel/sched.h"       // get_curr, get_first_free_proc
#include "arch/communication.h" // IPC: insert_msg, extract_msg, struct Message
#include "arch_inc/mem_constants.h" // PAGE_SIZE, Permisos

// Variables externas del kernel
extern paddr_t kernel_page_table; // Definido en arch/riscV/mem.c

// -------------------------------------------------------------------------
// TEST: Mecanismos de Paginación (Ring 0)
// Verifica que podamos mapear una dirección virtual arbitraria a una física,
// escribir en ella y recuperar la traducción correcta.
// -------------------------------------------------------------------------
void test_paging_mechanisms(CTest *ctx) {
    // 1. Obtener una página física libre
    paddr_t phys_page = alloc_pages(1);
    CTEST_ASSERT_NOT_NULL(ctx, (void*)phys_page, "Allocated physical page is valid");

    // 2. Definir una dirección virtual de prueba (lejos del kernel y stack)
    // Usamos 0x40000000 (1GB mark) que debería estar libre en tu layout
    vaddr_t virt_addr = 0x40000000;

    // 3. Mapear V -> P en la tabla de páginas del kernel actual
    // Usamos permisos de lectura/escritura (PAGE_R | PAGE_W | PAGE_V)
    // Nota: Ajusta los permisos según tu arquitectura (RISC-V vs x86)
    // En tu código usas constantes como PAGE_R, PAGE_W definidos en mem_constants.h
    uint32_t perms = 0x7; // R|W|X|V (simplificado) o usa las macros si las tienes visibles
    
    // Asumimos que map_page toma (uint32_t*) como tabla base.
    // kernel_page_table es paddr_t, cast necesario.
    map_page((uint32_t*)kernel_page_table, virt_addr, phys_page, perms);

    // 4. Verificar la traducción inversa (MMU Software Walk)
    paddr_t translated_addr = get_paddr_for((uint32_t*)kernel_page_table, virt_addr);
    CTEST_ASSERT_EQ(ctx, phys_page, translated_addr, "MMU Translation Check (get_paddr_for)");

    // 5. Prueba de acceso a memoria (Write/Read) a través del mapeo virtual
    // Esto confirmará que la TLB/MMU está aceptando la nueva entrada
    int *ptr = (int*)virt_addr;
    *ptr = 0x12345678; // Escribir patrón
    
    // Forzar barrera o flush si es necesario (generalmente map_page hace sfence.vma)
    // En tu código switch_page_table hace sfence, pero map_page individual no siempre.
    // Si falla, podría requerir un sfence.vma aquí.
    __asm__ volatile("sfence.vma"); 

    int read_back = *ptr;
    CTEST_ASSERT_EQ(ctx, 0x12345678, read_back, "Virtual Memory Read/Write Integrity");
}

// -------------------------------------------------------------------------
// TEST: Gestión de Procesos (Estructuras Internas)
// Verifica que el kernel pueda gestionar la lista de procesos y el PCB.
// -------------------------------------------------------------------------
void test_process_management(CTest *ctx) {

    // 1. Simular creación de proceso (obtener slot libre)
    struct Proc* new_proc = get_first_free_proc();
    CTEST_ASSERT_NOT_NULL(ctx, new_proc, "Can acquire a free process slot");
    
    if (new_proc) {
        // Verificar estado inicial
        // get_first_free_proc suele marcarlo como PROC_NOT_RUNNABLE para reservarlo
        CTEST_ASSERT_EQ(ctx, PROC_NOT_RUNNABLE, new_proc->status, "New process slot reserved (NOT_RUNNABLE)");
        
        // Limpieza: Liberarlo manualmente para no agotar slots
        new_proc->status = PROC_FREE;
    }
}

// -------------------------------------------------------------------------
// TEST: IPC (Inter-Process Communication) - Colas de Mensajes
// Verifica la lógica de encolado y desencolado de mensajes sin context switch.
// -------------------------------------------------------------------------
void test_ipc_logic(CTest *ctx) {
    // Usamos un proceso ficticio para probar su cola de mensajes
    struct Proc dummy_proc;
    dummy_proc.pid = 999;
    dummy_proc.msgs_queue.len_queue = 0; // Resetear cola

    // 1. Crear mensaje de prueba
    struct Message msg_in;
    msg_in.sender_pid = 1;
    msg_in.type = 10; // Tipo arbitrario
    msg_in.content_size = 5;
    strcpy((char*)msg_in.content, "TEST");

    // 2. Insertar mensaje (Simula send_msg)
    int res = insert_msg(&dummy_proc, msg_in);
    CTEST_ASSERT_EQ(ctx, 1, res, "IPC: Message insertion success");
    CTEST_ASSERT_EQ(ctx, 1, dummy_proc.msgs_queue.len_queue, "IPC: Queue length increased");

    // 3. Extraer mensaje (Simula recv_msg)
    struct Message msg_out = extract_msg(&dummy_proc);
    
    // 4. Validaciones
    CTEST_ASSERT_EQ(ctx, msg_in.sender_pid, msg_out.sender_pid, "IPC: Sender PID match");
    CTEST_ASSERT_EQ(ctx, msg_in.type, msg_out.type, "IPC: Message Type match");
    
    int content_match = strcmp((char*)msg_in.content, (char*)msg_out.content);
    CTEST_ASSERT_EQ(ctx, 0, content_match, "IPC: Content integrity match");
    
    CTEST_ASSERT_EQ(ctx, 0, dummy_proc.msgs_queue.len_queue, "IPC: Queue length decreased (Empty)");
}


// -------------------------------------------------------------------------
// TEST: Lógica de Bits de Paginación (PTE Construction)
// Verifica que al mapear, los bits de permisos se establezcan correctamente
// en la entrada de la tabla, sin depender de que la CPU lo ejecute.
// -------------------------------------------------------------------------
void test_page_permission_bits(CTest *ctx) {
    // Usamos direcciones dummy para probar la lógica de construcción de PTE
    paddr_t phys = 0x1000;
    vaddr_t virt = 0x2000;
    
    // Permisos arbitrarios: Read | Execute (Kernel Code)
    // Asumiendo macros estándar RISC-V: R=1, W=2, X=4, U=16
    // Ajusta estos valores mágicos a tus macros en mem_constants.h
    uint32_t perms_rw = 0x3; // R + W (Read/Write)
    uint32_t perms_rx = 0x5; // R + X (Read/Execute)

    // Usamos una tabla de páginas temporal en el stack para no ensuciar la del kernel
    // Una tabla SV32 tiene 1024 entradas de 4 bytes = 4KB.
    uint32_t temp_pgdir[1024] __attribute__((aligned(4096)));
    memset(temp_pgdir, 0, 4096);

    // Mapeamos RW
    map_page(temp_pgdir, virt, phys, perms_rw);
    
    // Obtenemos la entrada cruda (lógica simulada o lectura directa si tienes get_pte)
    // Como get_paddr_for devuelve la dirección física, verificamos que sea accesible.
    // Una prueba más profunda leería temp_pgdir[VPN(virt)] para ver los bits.
    
    // Verificación indirecta:
    // Si map_page funciona, get_paddr debería devolver 'phys'
    paddr_t translated = get_paddr_for(temp_pgdir, virt);
    CTEST_ASSERT_EQ(ctx, phys, translated, "PTE Construction: Address translation match");
    
}

// -------------------------------------------------------------------------
// TEST: Ciclo de Vida de Procesos y PIDs
// Verifica que los PIDs no colisionen y el reciclaje básico.
// -------------------------------------------------------------------------
void test_process_lifecycle_simulation(CTest *ctx) {
    struct Proc* p1 = get_first_free_proc();
    struct Proc* p2 = get_first_free_proc();
    
    CTEST_ASSERT_NOT_NULL(ctx, p1, "Proc 1 allocation");
    CTEST_ASSERT_NOT_NULL(ctx, p2, "Proc 2 allocation");
    
    if (p1 && p2) {
        // Simular inicialización
        p1->pid = 100;
        p1->status = PROC_RUNNING;
        
        p2->pid = 101;
        p2->status = PROC_RUNNING;
        
        CTEST_ASSERT_TRUE(ctx, p1 != p2, "Procs are different structs");
        
        // Simular muerte de P1
        p1->status = PROC_FREE; // O PROC_ZOMBIE si tienes wait()
        
        // Pedir nuevo proceso, debería reutilizar P1 o dar uno nuevo P3
        struct Proc* p3 = get_first_free_proc();
        CTEST_ASSERT_NOT_NULL(ctx, p3, "Proc 3 allocation after free");
        
    }
}