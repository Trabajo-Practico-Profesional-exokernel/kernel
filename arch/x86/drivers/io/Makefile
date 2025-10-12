BUILD_DIR = ../../../../build

IO_SRC := $(wildcard *.s)
IO_OBJ := $(patsubst %.s, $(BUILD_DIR)/%.o, $(IO_SRC))

all: $(IO_OBJ)

$(BUILD_DIR)/%.o: %.s
	nasm -f elf $< -o $@