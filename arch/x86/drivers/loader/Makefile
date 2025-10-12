BUILD_DIR = ../../../../build

LOADER_SRC := $(wildcard *.s)
LOADER_OBJ := $(patsubst %.s, $(BUILD_DIR)/%.o, $(LOADER_SRC))

all: $(LOADER_OBJ)

$(BUILD_DIR)/%.o: %.s
	nasm -f elf $< -o $@