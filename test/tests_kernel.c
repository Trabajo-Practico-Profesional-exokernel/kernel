#include "tests_kernel.h"
#include "test_common.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "utils.h"
// Includes internos del kernel necesarios
#include "arch/mem.h"           // alloc_pages, map_page, get_paddr_for
#include "arch/proc.h"          // struct Proc
#include "../kernel/core/sched.h"       // get_curr, get_first_free_proc
#include "arch/communication.h" // IPC: insert_msg, extract_msg, struct Message
#include "arch_inc/mem_constants.h" // PAGE_SIZE, Permisos

// Variables externas del kernel
extern paddr_t kernel_page_table; // Definido en arch/riscV/mem.c

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
/*void test_ipc_logic(CTest *ctx) {
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
}*/



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