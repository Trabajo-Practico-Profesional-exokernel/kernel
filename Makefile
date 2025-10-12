
BUILD_DIR = build
BOOTLOADER = arch/x86/drivers/loader
KERNEL_ELF = arch/x86/drivers/linker
KERNEL_SRC = kernel
IO_DRIVER = arch/x86/drivers/io
IO = drivers

OBJECTS = $(BOOTLOADER) $(KERNEL_ELF) $(KERNEL_SRC) $(IO_DRIVER) $(IO)
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
			-nostartfiles -nodefaultlibs -Wall -Wextra -c
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

build:
	mkdir -p build

all: kernel.elf kernel_src

bootloader: build
	make -C $(BOOTLOADER)

kernel.elf: bootloader kernel_src io
	make -C $(KERNEL_ELF)

io:
	make -C $(IO_DRIVER)
	make -C $(IO)

kernel_src:
	make -C $(KERNEL_SRC)

os.iso: kernel.elf
	cp build/kernel.elf arch/x86/iso/boot/kernel.elf
	
	genisoimage -R \
		-b boot/grub/stage2_eltorito \
		-no-emul-boot \
		-boot-load-size 4 \
		-A os \
		-input-charset utf8 \
		-quiet \
		-boot-info-table \
		-o os.iso \
		arch/x86/iso  


run: os.iso
	qemu-system-i386 -cdrom os.iso -boot d -m 32

clean:
	rm -rf build/*
