#include "tests_hardware.h"
#include "test_common.h"
#include "std/printf.h"
#include "arch/mem.h"
#include "std/string.h"
#include "utils.h"

// Includes específicos de arquitectura RISC-V
#include "arch_inc/virtio.h"
#include "arch_inc/virtio_blk.h"
#include "arch_inc/trap_constants.h"

#define VIRTIO_MAGIC_EXPECTED 0x74726976 
#define TEST_DISK_SECTOR 200

extern char __kernel_base[];
extern char _end[];
extern paddr_t kernel_page_table;

// -------------------------------------------------------------------------
// TEST 1: Integridad de VirtIO (MMIO)
// -------------------------------------------------------------------------
void test_virtio_integrity(CTest *ctx) {
    // 1. Verificar Magic Number
    uint32_t magic_val = virtio_reg_read32(VIRTIO_REG_MAGIC);
    CTEST_ASSERT_EQ(ctx, VIRTIO_MAGIC_EXPECTED, magic_val, "VirtIO Magic Number Check");
    
    // 2. Verificar Versión (Legacy/Transitional debe ser 1)
    uint32_t version = virtio_reg_read32(VIRTIO_REG_VERSION);
    CTEST_ASSERT_EQ(ctx, 1, version, "VirtIO Device Version Check");
    
    // 3. Verificar Device ID (Block Device debe ser 2)
    uint32_t device_id = virtio_reg_read32(VIRTIO_REG_DEVICE_ID);
    CTEST_ASSERT_EQ(ctx, 2, device_id, "VirtIO Device ID is Block Device");
}

// -------------------------------------------------------------------------
// TEST 2: Timer del CPU (CSR)
// -------------------------------------------------------------------------
void test_timer_csr(CTest *ctx) {
    uint32_t t1 = r_time();
    
    // Busy wait
    for(volatile int i = 0; i < 10000; i++); 
    
    uint32_t t2 = r_time();

    // Verificamos que el tiempo avanzó (t2 > t1)
    CTEST_ASSERT_GT(ctx, t2, t1, "RISC-V Timer (CSR) increment check");
    
    // Verificamos que t2 no sea 0 (sanity check)
    CTEST_ASSERT_GT(ctx, t2, 0, "Timer value is not zero");
}

// -------------------------------------------------------------------------
// TEST 3: Disco I/O (Lectura/Escritura)
// -------------------------------------------------------------------------
void test_disk_loopback(CTest *ctx) {
    char write_buf[SECTOR_SIZE];
    char read_buf[SECTOR_SIZE];
    const char* pattern = "HARDWARE_TEST_PATTERN_XYZ";

    // Limpieza de buffers
    memset(write_buf, 0, SECTOR_SIZE);
    memset(read_buf, 0, SECTOR_SIZE);
    strcpy(write_buf, pattern);

    // 1. Escribir al disco
    int write_res = read_write_disk(write_buf, TEST_DISK_SECTOR, 1);
    CTEST_ASSERT_EQ(ctx, 0, write_res, "VirtIO Disk Write Return Code");

    // Si falló la escritura, no tiene sentido seguir, pero el framework lo registrará
    if (write_res != 0) return; 

    // 2. Leer del disco
    int read_res = read_write_disk(read_buf, TEST_DISK_SECTOR, 0);
    CTEST_ASSERT_EQ(ctx, 0, read_res, "VirtIO Disk Read Return Code");

    // 3. Comparar contenido
    int cmp = strcmp(read_buf, pattern);
    CTEST_ASSERT_EQ(ctx, 0, cmp, "Disk Data Integrity (strcmp)");
    
    // 4. Verificar longitud (doble chequeo)
    int len_read = strlen(read_buf);
    int len_expected = strlen(pattern);
    CTEST_ASSERT_EQ(ctx, len_expected, len_read, "Disk Data Length Check");
}

// -------------------------------------------------------------------------
// TEST 4: Integridad de Registros CSR (sepc)
// Escribe un valor en un registro de control seguro y lo lee de vuelta.
// -------------------------------------------------------------------------
void test_csr_sepc_rw(CTest *ctx) {
    uint32_t original_sepc = READ_CSR(sepc); // Guardar valor original
    uint32_t test_val = 0xCAFEBABE;

    WRITE_CSR(sepc, test_val);
    uint32_t read_back = READ_CSR(sepc);

    CTEST_ASSERT_EQ(ctx, test_val, read_back, "CSR 'sepc' Read/Write Integrity");

    WRITE_CSR(sepc, original_sepc); // Restaurar valor original por seguridad
}

// -------------------------------------------------------------------------
// TEST 5: Integridad de MMU (Paging)
// Verifica que la dirección virtual del Kernel traduce a la física correcta.
// -------------------------------------------------------------------------
void test_mmu_kernel_mapping(CTest *ctx) {
    // La dirección virtual del inicio del kernel
    vaddr_t kbase_vaddr = (vaddr_t)__kernel_base;
    
    // Obtenemos la dirección física real mirando la tabla de páginas del kernel
    paddr_t mapped_paddr = get_paddr_for((uint32_t*)kernel_page_table, kbase_vaddr);

    // Virt 0x80200000 debe mapear a Phys 0x80200000.
    
    CTEST_ASSERT_EQ(ctx, (paddr_t)kbase_vaddr, mapped_paddr, "MMU Kernel Base Identity Mapping");
    
    // Verificamos también que NO devuelva 0 (que significaría no mapeado)
    CTEST_ASSERT_NOT_NULL(ctx, (void*)mapped_paddr, "Kernel Page Table Entry exists");
}

// -------------------------------------------------------------------------
// TEST 6: Estado de Interrupciones (sstatus)
// Verifica que el bit SIE (Supervisor Interrupt Enable) esté activo.
// -------------------------------------------------------------------------
void test_sstatus_interrupts(CTest *ctx) {
    uint32_t sstatus = READ_CSR(sstatus);
    
    // Bit 1 de sstatus es SIE (Supervisor Interrupt Enable)
    int sie_bit = (sstatus >> 1) & 1;
    
    CTEST_ASSERT_TRUE(ctx, sie_bit, "CPU Supervisor Interrupts (SIE) are Enabled");
}

// -------------------------------------------------------------------------
// TEST 7: Verificación de Modo de Paginación (SATP)
// Verifica que la paginación esté realmente activa en el hardware (Mode bit).
// En RV32, el bit 31 de satp controla el modo (1 = Sv32, 0 = Bare).
// -------------------------------------------------------------------------
void test_mmu_enabled_satp(CTest *ctx) {
    uint32_t satp_val = READ_CSR(satp);
    
    // Extraer el bit más significativo (Bit 31 para RV32)
    // Nota: Si estuvieras en RV64, el modo son los bits 60-63.
    int mode_bit = (satp_val >> 31) & 1;
    
    CTEST_ASSERT_TRUE(ctx, mode_bit, "MMU Paging Enabled (SATP Mode bit is 1)");
}

// -------------------------------------------------------------------------
// TEST 8: Alineación del Stack Pointer (ABI RISC-V)
// La especificación RISC-V exige que el SP esté siempre alineado a 16 bytes.
// Si esto falla, las llamadas a funciones C pueden corromper la memoria.
// -------------------------------------------------------------------------
void test_stack_alignment(CTest *ctx) {
    uintptr_t current_sp;
    
    // Leer el registro sp directamente usando ensamblador inline
    __asm__ volatile("mv %0, sp" : "=r"(current_sp));
    
    // Verificar alineación de 16 bytes (sp % 16 == 0)
    int alignment_mod = current_sp & 0xF; // 0xF es 15 (1111 binario)
    
    CTEST_ASSERT_EQ(ctx, 0, alignment_mod, "Stack Pointer 16-byte Alignment");
}

// -------------------------------------------------------------------------
// TEST 9: Configuración del Trap Vector (stvec)
// Verifica que el handler de excepciones esté configurado y alineado a 4 bytes.
// -------------------------------------------------------------------------
void test_trap_vector_config(CTest *ctx) {
    uint32_t stvec_val = READ_CSR(stvec);
    
    // 1. Verificar que no sea NULL (debe apuntar a tu función trap_entry o similar)
    CTEST_ASSERT_NOT_NULL(ctx, (void*)stvec_val, "Trap Vector (stvec) is set");
    
    // 2. Verificar alineación.
    // Asumiendo kernel base 0x80200000
    CTEST_ASSERT_GE(ctx, stvec_val, 0x80000000, "Trap Vector points to Kernel Space");
}
