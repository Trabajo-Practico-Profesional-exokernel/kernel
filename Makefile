# ============================
# Configuración general
# ============================

ARCH        ?= x86                        # o riscv
BUILD_DIR   = build/$(ARCH)
INC_DIR     = public

# ----------------------------
# Compiladores por arquitectura
# ----------------------------
# -no-reboot -no-shutdown

ifeq ($(ARCH),x86)
	CC      = gcc
	AS      = nasm
	CFLAGS  = -I$(INC_DIR) -Iarch/x86 -Iarch/x86/drivers -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	           -nostartfiles -nodefaultlibs -Wall -Wextra -c -g -DIS_X86
	ASFLAGS = -f elf
	LDFLAGS = -T arch/x86/drivers/linker/link.ld -melf_i386
	QEMU    = qemu-system-i386 -cdrom os.iso  -m 64 -no-reboot -no-shutdown -nographic -serial mon:stdio
else ifeq ($(ARCH),riscv)
	CC      = clang
	CFLAGS  = -I$(INC_DIR) -Iarch/riscV -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf \
	           -fno-stack-protector -ffreestanding -nostdlib -DIS_RISC

	# If riscv then add -DIS_RISC that deifines the constant IS_RISC for conditional compiling
	
	LDFLAGS = -T arch/riscV/linker/link.ld
	QEMU    = qemu-system-riscv32 -machine virt -bios default -nographic -serial mon:stdio --no-reboot -kernel $(BUILD_DIR)/kernel.elf
endif

# ============================
# Directorios fuente
# ============================

ifeq ($(ARCH),x86)
	SRC_DIRS = arch/x86 arch/x86/drivers/io arch/x86/drivers/loader kernel
else ifeq ($(ARCH),riscv)
	SRC_DIRS = arch/riscV/drivers arch/riscV kernel user
endif

# Buscar fuentes (.c y .s)
C_SOURCES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_SOURCES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))
SOURCES   := $(C_SOURCES) $(S_SOURCES)

# Generar objetos bajo build/<arch>/... (replica la estructura original)
OBJECTS := $(patsubst %,$(BUILD_DIR)/%,$(SOURCES))
OBJECTS := $(OBJECTS:.c=.o)
OBJECTS := $(OBJECTS:.s=.o)

#Empty
USER_OBJECTS:=
ifeq ($(ARCH),riscv)
	USER_BUILD_FOLDER:=apps/build
	USER_OBJECTS := $(wildcard $(USER_BUILD_FOLDER)/*.o)
endif

# ============================
# Reglas principales
# ============================

.PHONY: all run clean os.iso

all: $(BUILD_DIR)/kernel.elf
ifeq ($(ARCH),x86)
all: os.iso
endif

# Crear carpetas necesarias (con subdirectorios)
$(BUILD_DIR):
	mkdir -p $(sort $(dir $(OBJECTS)))

# Link final
$(BUILD_DIR)/kernel.elf: $(OBJECTS)
ifeq ($(ARCH),x86)
	ld $(LDFLAGS) $(OBJECTS) -o $@
else ifeq ($(ARCH),riscv)
	$(CC) $(CFLAGS) $(OBJECTS) $(USER_OBJECTS) -Wl,$(LDFLAGS) -o $@
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
	cp $(BUILD_DIR)/kernel.elf arch/x86/iso/boot/kernel.elf
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
else
	@echo "⚠️  os.iso solo aplica para x86"
endif

# ============================
# Ejecución
# ============================

run: all
	$(QEMU)
debug: all
	$(QEMU) -boot d -gdb tcp::26000 -S

# ============================
# Limpieza
# ============================

clean:
	rm -rf build *.iso

#debug: all
#	qemu-system-i386 -cdrom os.iso -boot d -gdb tcp::26000 -S