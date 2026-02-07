# ============================
# Configuración general
# ============================

ARCH        ?= x86
BUILD_DIR   = build/$(ARCH)
# CAMBIO: Agregado -Ikernel/core para mantener dependencias de headers
DEF_INCS     = -Ipublic -Isys -Iutil -Ilibc -Ikernel/core

KERNEL_DISK_PATH ?=.kernel_disk/disk.txt

NCPU ?= 1
CPU_STACK_PAGES = 32
CPU_TRAP_STACK_PAGES = 32
TESTING ?= 0

KERNEL_MAIN := kernel/core/kmain.c


# ----------------------------
# Compiladores por arquitectura
# ----------------------------

ifeq ($(ARCH),x86)
	CC      = gcc
	AS      = nasm
	CFLAGS  = $(DEF_INCS) -Ikernel/arch/x86 -Ikernel/arch/x86/drivers -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	           -nostartfiles -nodefaultlibs -Wall -Wextra -c -g -DIS_X86 -march=i386 -mtune=i386\
	                   -DNCPU=$(NCPU)
	TEST_DIRS := test/core test/x86
	TEST_INCS := -Itest/core -Itest/x86

	ASFLAGS = -f elf
	LDFLAGS = -T kernel/arch/x86/drivers/linker/link.ld -melf_i386
	QEMU    = qemu-system-i386 -cdrom os.iso  -m 64 -no-reboot -no-shutdown -nographic -serial mon:stdio \
	                                                        -drive file=$(KERNEL_DISK_PATH),index=1,media=disk,format=raw
else ifeq ($(ARCH),riscv)
	CC      = clang

	TEST_DIRS := test/core test/riscv
	TEST_INCS := -Itest/core -Itest/riscv

	CFLAGS  = $(DEF_INCS) -Ikernel/arch/riscV -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf \
	           -fno-stack-protector -ffreestanding -nostdlib -DIS_RISC -fno-pic -fno-pie -mcmodel=medany\
	                           -DNCPU=$(NCPU) \
	                           -DCPU_STACK_PAGES=$(CPU_STACK_PAGES) \
	                           -DCPU_TRAP_STACK_PAGES=$(CPU_TRAP_STACK_PAGES) \

	LDFLAGS = -T kernel/arch/riscV/linker/link.ld \
	  -Wl,--defsym=NCPU=$(NCPU) \
	  -Wl,--defsym=CPU_STACK_PAGES=$(CPU_STACK_PAGES) \
	  -Wl,--defsym=CPU_TRAP_STACK_PAGES=$(CPU_TRAP_STACK_PAGES)

	QEMU    = qemu-system-riscv32 -machine virt -bios default -nographic -serial mon:stdio --no-reboot -kernel $(BUILD_DIR)/kernel.elf \
	                                                        -smp $(NCPU) -drive id=drive0,file=$(KERNEL_DISK_PATH),format=raw,if=none \
	                                                    -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0
endif

# ============================
# Directorios fuente
# ============================

# CAMBIO: Reemplazado 'kernel' por 'kernel/core' en ambas arquitecturas
ifeq ($(ARCH),x86)
	SRC_DIRS = kernel/arch/x86 kernel/arch/x86/drivers/io kernel/arch/x86/drivers/loader kernel/core sys libc util util/console util/parsers kernel/arch/x86/drivers
else ifeq ($(ARCH),riscv)
	SRC_DIRS = kernel/arch/riscV/drivers kernel/arch/riscV kernel/core sys libc util util/console util/parsers
endif

C_SOURCES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_SOURCES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))


ifeq ($(TESTING),1)
	# Remove real kernel main
	C_SOURCES := $(filter-out $(KERNEL_MAIN),$(C_SOURCES))
	
	C_SOURCES += $(shell find $(TEST_DIRS) -type f -name '*.c')
#user/meta/app_names.c Not needed as of now since have access

# 	C_SOURCES += $(foreach dir,$(TEST_DIRS),$(wildcard $(dir)/*.c))
	CFLAGS += $(TEST_INCS) -DIS_TESTING
endif


SOURCES   := $(C_SOURCES) $(S_SOURCES)




OBJECTS := $(patsubst %,$(BUILD_DIR)/%,$(SOURCES))
OBJECTS := $(OBJECTS:.c=.o)
OBJECTS := $(OBJECTS:.s=.o)

USER_APPS_OBJECTS :=
USER_BUILD_FOLDER := user/build
USER_APPS_OBJECTS := $(wildcard $(USER_BUILD_FOLDER)/**/*.o)

# ============================
# Reglas principales
# ============================

.PHONY: all run clean os.iso

all: $(BUILD_DIR)/kernel.elf
ifeq ($(ARCH),x86)
all: os.iso
endif

$(BUILD_DIR):
	mkdir -p $(sort $(dir $(OBJECTS)))

$(BUILD_DIR)/kernel.elf: $(OBJECTS)
ifeq ($(ARCH),x86)
	$(info Linking with user apps: $(USER_APPS_OBJECTS))
	ld $(LDFLAGS) $(OBJECTS) $(USER_APPS_OBJECTS) -o $@
else ifeq ($(ARCH),riscv)
	$(info Linking with user apps: $(USER_APPS_OBJECTS))
	$(CC) $(CFLAGS) $(OBJECTS) $(USER_APPS_OBJECTS) -Wl,$(LDFLAGS) -o $@
endif

# ============================
# Compilación genérica
# ============================

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	echo "Compilando C: $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	echo "Ensamblando: $< -> $@"
ifeq ($(ARCH),x86)
	$(AS) $(ASFLAGS) $< -o $@
else
	$(CC) $(CFLAGS) -c $< -o $@
endif

# ============================
# ISO (solo x86)
# ============================

os.iso: $(BUILD_DIR)/kernel.elf
ifeq ($(ARCH),x86)
	cp $(BUILD_DIR)/kernel.elf kernel/arch/x86/iso/boot/kernel.elf
	genisoimage -R \
	        -b boot/grub/stage2_eltorito \
	        -no-emul-boot \
	        -boot-load-size 4 \
	        -A os \
	        -input-charset utf8 \
	        -quiet \
	        -boot-info-table \
	        -o os.iso \
	        kernel/arch/x86/iso
else
	@echo "⚠️  os.iso solo aplica para x86"
endif

# ============================
# Ejecución y Limpieza
# ============================
.kernel_disk/disk.txt:
	@mkdir -p .kernel_disk
	@dd if=/dev/zero of=.kernel_disk/disk.txt count=2048 bs=512

run: .kernel_disk/disk.txt all
	$(QEMU)
debug: all
	$(QEMU) -boot d -gdb tcp::26000 -S

clean:
	rm -rf build *.iso
