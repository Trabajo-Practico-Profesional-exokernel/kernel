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
    OBJFLAGS="-Oelf32-i386"
else
    ARCH_FOLDER=$ROOT/arch/riscV
    CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib -fno-pic -fno-pie -mcmodel=medany"
    OBJFLAGS="-Oelf32-littleriscv -B riscv"

fi

INC_DIR="-I$USER_FOLDER/lib -I$ROOT/public -I$ROOT/std -I$ROOT/sys -I$ROOT/util"  # Include .h from user lib, common public defs and general std
INC_DIR+=" -I$ARCH_FOLDER" # Also arch folder just in case?

## Add base src files
COMMON_SRC_FILES="$ARCH_FOLDER/user/entry_point.c"
COMMON_SRC_FILES+=" $(find "$ROOT/user/lib" -name "*.c")"
COMMON_SRC_FILES+=" $(find "$ROOT/std" -name "*.c")"
COMMON_SRC_FILES+=" $(find "$ROOT/util" -name "*.c")"



if [[ "$1" == "-add_meta" ]];then
    echo "Adding apps meta info to apps" 
    COMMON_SRC_FILES+=" $(find "$USER_FOLDER/meta" -name "*.c")"
    INC_DIR+=" -I$USER_FOLDER/meta"
    shift # Remove param
fi

mkdir -p $BUILD_FOLDER

# first build apps that have many .c files i.e have their own folder....
for app_dir in $@; do
    app_name=$(basename $app_dir)

    # Collect all .c files in the app directory
    SRC_FILES=$(find "$app_dir" -name "*.c")

    # Check if there are any .c files
    if [ -z "$SRC_FILES" ]; then
        echo "No .c files found in $app_dir, skipping."
        continue
    fi
    
    echo "$ARCH build '$app_name' "

    # Create build directories
    app_build_folder="$BUILD_FOLDER/$app_name"
    mkdir -p $app_build_folder

    # Build the app (ELF file)
    $CC $CFLAGS $INC_DIR -Wl,-T$ARCH_FOLDER/linker/user.ld -Wl,-Map=$app_build_folder/app.map -o $app_build_folder/app.elf $SRC_FILES $COMMON_SRC_FILES

    ## Convert ELF to binary
    $OBJCOPY --set-section-flags .bss=alloc,contents -O binary $app_build_folder/app.elf $app_build_folder/app.bin
    
    # Copiar a temporal para forzar nombre de simbolo limpio
    cp $app_build_folder/app.bin ${app_name}_app.bin
    $OBJCOPY -Ibinary $OBJFLAGS ${app_name}_app.bin $app_build_folder/app.bin.o
    rm ${app_name}_app.bin

    echo "'$app_name' build completed!"
done
