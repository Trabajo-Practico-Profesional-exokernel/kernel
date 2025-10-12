BUILD_DIR = ../../../../build
LOADER_OBJ = $(BUILD_DIR)/loader.o
LDFLAGS = -T link.ld -melf_i386
KERNEL_ELF = $(BUILD_DIR)/kernel.elf

OBJECTS = $(BUILD_DIR)/loader.o $(BUILD_DIR)/kmain.o $(BUILD_DIR)/fb.o $(BUILD_DIR)/io.o $(BUILD_DIR)/common.o

all: $(KERNEL_ELF)

$(KERNEL_ELF): $(LINKER_SRC) $(LOADER_OBJ)
	ld $(LDFLAGS) $(OBJECTS) -o $(KERNEL_ELF)
