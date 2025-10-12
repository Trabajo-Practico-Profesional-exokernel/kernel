BUILD_DIR = ../build

CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
			-nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c

KERNEL_SRC := $(wildcard *.c)
KERNEL_OBJ := $(patsubst %.c, $(BUILD_DIR)/%.o, $(KERNEL_SRC))

all: $(KERNEL_OBJ)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@