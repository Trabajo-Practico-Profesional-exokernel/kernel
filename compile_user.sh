#!/bin/bash


## First generate apps meta so that shell can access the indexs defined or so...
./generate_apps_meta.sh


OBJCOPY=llvm-objcopy  # or path to llvm-objcopy
QEMU=qemu-system-riscv32
CC=clang
INC_DIR=user_lib
ARCH_FOLDER=arch/riscV
BUILD_FOLDER=apps/build

mkdir -p $BUILD_FOLDER

CFLAGS="-I$INC_DIR -I$ARCH_FOLDER -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib"
CFLAGS+=" -Ipublic -Istd" 

COMMON_SRC_FILES="$ARCH_FOLDER/user/entry_point.c"
COMMON_SRC_FILES+=" $(find "user_lib" -name "*.c")"
COMMON_SRC_FILES+=" $(find "std" -name "*.c")"

echo "COMMON SRC FILES $COMMON_SRC_FILES"

# first build apps that have many .c files i.e have their own folder....
for app_dir in apps/*/; do

    app_name=$(basename $app_dir)
	if [[ "$app_name" == "build" ]];then
		continue
	fi


    
    # Create build directories
    app_build_folder="$BUILD_FOLDER/$app_name"
    mkdir -p $app_build_folder
    


    # Collect all .c files in the app directory
    SRC_FILES=$(find "$app_dir" -name "*.c")
    APP_CFLAGS=$CFLAGS
    # not the best? lol but works!
    if [[ "$app_name" == "shell" || "$app_name" == "tests_shell" ]];then
        #Be able to access to infor about what apps are there.. for shell basically
        SRC_FILES+=" $(find "meta/user_gen" -name "*.c")"
        APP_CFLAGS+=" -Imeta/user_gen" 
    fi

    # Check if there are any .c files
    if [ -z "$SRC_FILES" ]; then
        echo "No .c files found in $app_dir, skipping."
        continue
    fi
    echo "build '$app_name' with src files $SRC_FILES"

    # Build the app (ELF file)
    $CC $APP_CFLAGS -Wl,-T$ARCH_FOLDER/linker/user.ld -Wl,-Map=$app_build_folder/app.map -o $app_build_folder/app.elf $SRC_FILES $COMMON_SRC_FILES

    # # Convert ELF to binary
    $OBJCOPY --set-section-flags .bss=alloc,contents -O binary $app_build_folder/app.elf $app_build_folder/app.bin
    $OBJCOPY -Ibinary -Oelf32-littleriscv $app_build_folder/app.bin $app_build_folder/app.bin.o

    echo "'$app_name' build completed!"
done


### Then take the mono .c file like hello_world
for app_file in apps/*.c; do
    app_name=$(basename "$app_file" .c)
    echo "build '$app_name' with src files $app_file"

    # Create build directories
    app_build_folder="$BUILD_FOLDER/$app_name"
    mkdir -p $app_build_folder


    # Build the app (ELF file)
    $CC $CFLAGS -Wl,-T$ARCH_FOLDER/linker/user.ld -Wl,-Map=$app_build_folder/app.map -o $app_build_folder/app.elf $app_file $COMMON_SRC_FILES

    # # Convert ELF to binary
    $OBJCOPY --set-section-flags .bss=alloc,contents -O binary $app_build_folder/app.elf $app_build_folder/app.bin
    $OBJCOPY -Ibinary -Oelf32-littleriscv $app_build_folder/app.bin $app_build_folder/app.bin.o

    echo "'$app_name' build completed!"

done