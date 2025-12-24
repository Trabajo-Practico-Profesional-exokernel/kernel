#!/usr/bin/env bash
set -e

ARCH="$1"
shift #Remove arch param


ROOT=../
USER_FOLDER=$ROOT/user
BUILD_FOLDER=$USER_FOLDER/build
OBJCOPY=llvm-objcopy  # or path to llvm-objcopy
CC=clang

mkdir -p $BUILD_FOLDER



if [[ "$ARCH" == "x86" ]]; then
    ARCH_FOLDER=$ROOT/arch/x86
    CFLAGS="
        -std=c11
        -O2 -g3
        -Wall -Wextra
        --target=i386-unknown-elf
        -m32
        -ffreestanding
        -nostdlib
        -fno-stack-protector
        -fno-pic
        -fno-pie
        -no-pie
        -mno-red-zone
        -fno-asynchronous-unwind-tables
    "
    OBJFLAGS="-Oelf32-i386"
else
    ARCH_FOLDER=$ROOT/arch/riscV
    CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib"
    OBJFLAGS="-Oelf32-littleriscv"

fi

INC_DIR="-I$USER_FOLDER/lib -I$ROOT/public -I$ROOT/std"  # Include .h from user lib, common public defs and general std
INC_DIR+=" -I$ARCH_FOLDER" # Also arch folder just in case?

## Add base src files
COMMON_SRC_FILES="$ARCH_FOLDER/user/entry_point.c"
COMMON_SRC_FILES+=" $(find "$ROOT/user/lib" -name "*.c")"
COMMON_SRC_FILES+=" $(find "$ROOT/std" -name "*.c")"




# first build apps that have many .c files i.e have their own folder....
for app_file in $@; do
    app_name=$(basename "$app_file" .c)
    echo "$ARCH build '$app_name'"

    # Create build directories
    app_build_folder="$BUILD_FOLDER/$app_name"
    mkdir -p $app_build_folder

    # Build the app (ELF file)
    $CC $CFLAGS $INC_DIR -Wl,-T$ARCH_FOLDER/linker/user.ld -Wl,-Map=$app_build_folder/app.map -o $app_build_folder/app.elf $app_file $COMMON_SRC_FILES

    # # Convert ELF to binary
    $OBJCOPY --set-section-flags .bss=alloc,contents -O binary $app_build_folder/app.elf $app_build_folder/app.bin
    $OBJCOPY -Ibinary $OBJFLAGS $app_build_folder/app.bin $app_build_folder/app.bin.o

    echo "--- '$app_name' build completed!"

done
