#!/usr/bin/env bash
set -e

ARCH="$1"
shift #Remove arch param

ROOT=../
USER_FOLDER=$ROOT/user
BUILD_FOLDER=$USER_FOLDER/build
OBJCOPY=llvm-objcopy
CC=clang

mkdir -p $BUILD_FOLDER

if [[ "$ARCH" == "x86" ]]; then
    ARCH_FOLDER=$ROOT/kernel/arch/x86
    CFLAGS="
        -O2 -g3
        -Wall -Wextra
        --target=i386-unknown-elf
        -m32
        -ffreestanding
        -nostdlib
        -nostdinc
        -fno-stack-protector
        -fno-pic
        -fno-pie
        -no-pie
        -mno-red-zone
        -fno-asynchronous-unwind-tables
        -march=i386 -mtune=i386
        -g
    "
    OBJFLAGS="-Oelf32-i386 -B i386"
else
    ARCH_FOLDER=$ROOT/kernel/arch/riscV
    CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib -fno-pic -fno-pie -mcmodel=medany"
    OBJFLAGS="-Oelf32-littleriscv -B riscv"
fi

# CAMBIO: Agregado -I$ROOT/kernel/core
INC_DIR="-I$USER_FOLDER/lib -I$ROOT/public -I$ROOT/sys -I$ROOT/util -I$ROOT/libc -I$ROOT/kernel/core"
INC_DIR+=" -I$ARCH_FOLDER"

COMMON_SRC_FILES="$ARCH_FOLDER/user/entry_point.c"
COMMON_SRC_FILES+=" $(find "$ROOT/user/lib" -name "*.c")"
COMMON_SRC_FILES+=" $(find "$ROOT/libc" -name "*.c")"
COMMON_SRC_FILES+=" $(find "$ROOT/util" -name "*.c")"

for app_file in $@; do
    app_name=$(basename "$app_file" .c)
    echo "$ARCH build '$app_name'"

    app_build_folder="$BUILD_FOLDER/$app_name"
    mkdir -p $app_build_folder

    $CC $CFLAGS $INC_DIR -Wl,-T$ARCH_FOLDER/linker/user.ld -Wl,-Map=$app_build_folder/app.map -o $app_build_folder/app.elf $app_file $COMMON_SRC_FILES

    $OBJCOPY --set-section-flags .bss=alloc,contents -O binary $app_build_folder/app.elf $app_build_folder/app.bin
    
    cp $app_build_folder/app.bin ${app_name}_app.bin
    $OBJCOPY -Ibinary $OBJFLAGS ${app_name}_app.bin $app_build_folder/app.bin.o
    rm ${app_name}_app.bin

    echo "--- '$app_name' build completed!"
done
