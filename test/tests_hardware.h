#ifndef TESTS_HARDWARE_H
#define TESTS_HARDWARE_H

#include "test_common.h"

void test_virtio_integrity(CTest *ctx);
void test_timer_csr(CTest *ctx);
void test_disk_loopback(CTest *ctx);
void test_csr_sepc_rw(CTest *ctx);
void test_mmu_kernel_mapping(CTest *ctx);
void test_sstatus_interrupts(CTest *ctx);
void test_stack_alignment(CTest *ctx);
void test_trap_vector_config(CTest *ctx);


#endif