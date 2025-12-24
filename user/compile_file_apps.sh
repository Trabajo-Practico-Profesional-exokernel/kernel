#!/usr/bin/env bash
set -e

ROOT=..
ARCH_NAME=riscV

OBJCOPY=llvm-objcopy  # or path to llvm-objcopy
QEMU=qemu-system-riscv32
CC=clang



ARCH_FOLDER=$ROOT/arch/$ARCH_NAME

USER_FOLDER=$ROOT/user
BUILD_FOLDER=$USER_FOLDER/build

mkdir -p $BUILD_FOLDER

# Riscv CFLAGS
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib"


INC_DIR="-I$USER_FOLDER/lib -I$ROOT/public -I$ROOT/std"  # Include .h from user lib, common public defs and general std
INC_DIR+=" -I$ARCH_FOLDER" # Also arch folder just in case?

COMMON_SRC_FILES="$ARCH_FOLDER/user/entry_point.c" # Definition arch dependant for entry point and so on.. for servers it would change in the future
COMMON_SRC_FILES+=" $(find "$USER_FOLDER/lib" -name "*.c")" # User specific stdlib
COMMON_SRC_FILES+=" $(find "$ROOT/std" -name "*.c")" # General std lib for strings and so on.


# first build apps that have many .c files i.e have their own folder....
for app_file in $@; do
    app_name=$(basename "$app_file" .c)
    echo "build '$app_name' with src main file $app_file"

    # Create build directories
    app_build_folder="$BUILD_FOLDER/$app_name"
    mkdir -p $app_build_folder

    # Build the app (ELF file)
    $CC $CFLAGS -Wl,-T$ARCH_FOLDER/linker/user.ld -Wl,-Map=$app_build_folder/app.map -o $app_build_folder/app.elf $app_file $COMMON_SRC_FILES

    # # Convert ELF to binary
    $OBJCOPY --set-section-flags .bss=alloc,contents -O binary $app_build_folder/app.elf $app_build_folder/app.bin
    $OBJCOPY -Ibinary -Oelf32-littleriscv $app_build_folder/app.bin $app_build_folder/app.bin.o

    echo "--- '$app_name' build completed!"

done
