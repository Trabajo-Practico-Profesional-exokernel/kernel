BUILD_DIR = ../build

CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
			-nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c

FB_SRC := $(wildcard *.c)
FB_OBJ := $(patsubst %.c, $(BUILD_DIR)/%.o, $(FB_SRC))

all: $(FB_OBJ)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@