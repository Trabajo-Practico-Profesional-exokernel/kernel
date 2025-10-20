#!/bin/bash

OBJCOPY=llvm-objcopy #/opt/homebrew/opt/llvm/bin/llvm-objcopy

QEMU=qemu-system-riscv32

CC=clang
INC_DIR=public/user
ARCH_FOLDER=arch/riscV
BUILD_FOLDER=build/user
mkdir $BUILD_FOLDER

CFLAGS="-I$INC_DIR -I$ARCH_FOLDER -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib"


# No ideal agregar all public includes? o bue si? quien sabe ja.
CFLAGS+=" -Ipublic"
#-T arch/riscV/linker/user.ld
#-T arch/riscV/linker/user.ld

# Build the shell (application)
$CC $CFLAGS -Wl,-T$ARCH_FOLDER/linker/user.ld -Wl,-Map=$BUILD_FOLDER/shell.map -o $BUILD_FOLDER/shell.elf user/shell.c $ARCH_FOLDER/user/entry_point.c kernel/common.c
$OBJCOPY --set-section-flags .bss=alloc,contents -O binary $BUILD_FOLDER/shell.elf $BUILD_FOLDER/shell.bin
$OBJCOPY -Ibinary -Oelf32-littleriscv $BUILD_FOLDER/shell.bin $BUILD_FOLDER/shell.bin.o