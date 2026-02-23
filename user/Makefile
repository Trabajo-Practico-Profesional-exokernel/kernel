ROOT := ..
USER_FOLDER := $(ROOT)/user
ARCH        ?= riscv                        # or x86

APPS_FOLDER    := $(USER_FOLDER)/apps
DIR_APPS_FOLDER    := $(USER_FOLDER)/dir_apps
SERVERS_FOLDER := $(USER_FOLDER)/servers
BUILD_FOLDER   := $(USER_FOLDER)/build
APP_OUT        := $(BUILD_FOLDER)

TRG_DISK ?= $(ROOT)/.kernel_disk/disk.img


# 20MB
TRG_DISK_SIZE = $(shell echo $$((20 * 1024 * 1024)))

TRG_BASE_DISK ?= $(ROOT)/.kernel_disk/disk_base.img

# ----------------------------
# App discovery
# ----------------------------

# Servers (multi-file dirs)
SERVERS_DIRS := $(filter-out $(SERVERS_FOLDER)/build/, \
	$(wildcard $(SERVERS_FOLDER)/*/))

# General single-file apps (including those in subdirectories)
GENERAL_C := $(wildcard $(APPS_FOLDER)/**/*.c) $(wildcard $(APPS_FOLDER)/*.c)

# Extract relative paths from APPS_FOLDER (e.g., "utils/ls" from "apps/utils/ls.c")
GENERAL_C_REL := $(patsubst $(APPS_FOLDER)/%,%,$(GENERAL_C))

# Get target names by removing .c extension
GENERAL_C_NAMES := $(GENERAL_C_REL:.c=)

# General multi-file apps
GENERAL_DIRS := $(filter-out $(DIR_APPS_FOLDER)/build/, \
	$(wildcard $(DIR_APPS_FOLDER)/*/))


GENERAL_DIR_NAMES := $(notdir $(patsubst %/,%,$(GENERAL_DIRS)))
SERVERS_DIR_NAMES := $(notdir $(patsubst %/,%,$(SERVERS_DIRS)))

# ----------------------------
# Metadata app list (unchanged idea)
# ----------------------------

META_APP_NAMES := \
	$(SERVERS_DIR_NAMES) \
	$(GENERAL_C_NAMES) \
	$(GENERAL_DIR_NAMES)


MAIN_APP_NAMES := shell

META_APP_NAMES += $(MAIN_APP_NAMES)
META_APP_NAMES := $(sort $(META_APP_NAMES))

DEF_EXECUTABLES := touch rm stat cat

# Ensure default executables are first
META_APP_NAMES := $(filter-out $(DEF_EXECUTABLES),$(META_APP_NAMES))
META_APP_NAMES := $(strip \
	$(DEF_EXECUTABLES) \
	$(META_APP_NAMES))

DEF_EXECUTABLE_COUNT := $(words $(DEF_EXECUTABLES))

# ----------------------------
# Build outputs
# ----------------------------

SERVER_BINS      := $(addprefix $(APP_OUT)/,$(SERVERS_DIR_NAMES))
GENERAL_C_BINS   := $(addprefix $(APP_OUT)/,$(GENERAL_C_NAMES))
GENERAL_DIR_BINS := $(addprefix $(APP_OUT)/,$(GENERAL_DIR_NAMES))

ALL_BINS := \
	$(SERVER_BINS) \
	$(GENERAL_C_BINS) \
	$(GENERAL_DIR_BINS)

# ----------------------------
# Top-level targets
# ----------------------------

.PHONY: all clean metadata apps

all: $(APP_OUT) metadata $(ALL_BINS)
	@echo "Building main apps (shell)"
	@./compile_dir_apps.sh $(ARCH) -add_meta $(MAIN_APP_NAMES)

	@python3 disk_image_builder.py $(BUILD_FOLDER) $(TRG_BASE_DISK) $(TRG_DISK_SIZE)
	cp $(TRG_BASE_DISK) $(TRG_DISK) 
apps:
	@echo $(META_APP_NAMES)
disk_image:
	@python3 disk_image_builder.py $(BUILD_FOLDER) $(TRG_BASE_DISK) $(TRG_DISK_SIZE)
app:
	@if [ -z "$(APP)" ]; then \
		echo "Usage: make app APP=name"; \
		exit 1; \
	fi
	@$(MAKE) $(APP_OUT)/$(APP)
	@python3 disk_image_builder.py $(BUILD_FOLDER) $(TRG_BASE_DISK) $(TRG_DISK_SIZE)
	
shell:
	@./compile_dir_apps.sh $(ARCH) -add_meta shell
	@python3 disk_image_builder.py $(BUILD_FOLDER) $(TRG_BASE_DISK) $(TRG_DISK_SIZE)
	


# ----------------------------
# Metadata
# ----------------------------

metadata:
	@./generate_apps_meta.sh $(DEF_EXECUTABLE_COUNT) $(META_APP_NAMES)

# ----------------------------
# Pattern rules (the important part)
# ----------------------------

# Single-file apps (including those in subdirectories)
$(APP_OUT)/%: $(APPS_FOLDER)/%.c | $(APP_OUT)
	@mkdir -p $(dir $@)
	@echo "Compiling single-file app: $*"
	@./compile_file_apps.sh $(ARCH) $<

# Multi-file general apps
$(APP_OUT)/%: $(DIR_APPS_FOLDER)/%/ | $(APP_OUT)
	@echo "Compiling dir app: $*"
	@./compile_dir_apps.sh $(ARCH) $<

# Server apps
$(APP_OUT)/%: $(SERVERS_FOLDER)/%/ | $(APP_OUT)
	@echo "Compiling server: $*"
	@./compile_dir_apps.sh $(ARCH) -add_meta $<

# ----------------------------
# Directories
# ----------------------------

$(APP_OUT):
	@mkdir -p $(APP_OUT)

# ----------------------------
# Cleanup
# ----------------------------

clean:
	rm -rf $(BUILD_FOLDER)
