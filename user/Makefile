ROOT := ..
USER_FOLDER := $(ROOT)/user
ARCH        ?= riscv                        # or x86

APPS_FOLDER    := $(USER_FOLDER)/apps
SERVERS_FOLDER := $(USER_FOLDER)/servers
BUILD_FOLDER   := $(USER_FOLDER)/build
APP_OUT        := $(BUILD_FOLDER)

# ----------------------------
# App discovery
# ----------------------------

# Servers (multi-file dirs)
SERVERS_DIRS := $(filter-out $(SERVERS_FOLDER)/build/, \
	$(wildcard $(SERVERS_FOLDER)/*/))

# General single-file apps
GENERAL_C := $(wildcard $(APPS_FOLDER)/*.c)

# General multi-file apps
GENERAL_DIRS := $(filter-out $(APPS_FOLDER)/build/, \
	$(wildcard $(APPS_FOLDER)/*/))

GENERAL_C_NAMES   := $(basename $(notdir $(GENERAL_C)))
GENERAL_DIR_NAMES := $(notdir $(patsubst %/,%,$(GENERAL_DIRS)))
SERVERS_DIR_NAMES := $(notdir $(patsubst %/,%,$(SERVERS_DIRS)))

# ----------------------------
# Metadata app list (unchanged idea)
# ----------------------------

META_APP_NAMES := \
	$(SERVERS_DIR_NAMES) \
	$(GENERAL_C_NAMES) \
	$(GENERAL_DIR_NAMES)

META_APP_NAMES += shell tests_shell
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
	@./compile_dir_apps.sh $(ARCH) -add_meta shell tests_shell

apps:
	@echo $(META_APP_NAMES)

# ----------------------------
# Metadata
# ----------------------------

metadata:
	@./generate_apps_meta.sh $(DEF_EXECUTABLE_COUNT) $(META_APP_NAMES)

# ----------------------------
# Pattern rules (the important part)
# ----------------------------

# Single-file apps
$(APP_OUT)/%: $(APPS_FOLDER)/%.c | $(APP_OUT)
	@echo "Compiling single-file app: $*"
	@./compile_file_apps.sh $(ARCH) $<

# Multi-file general apps
$(APP_OUT)/%: $(APPS_FOLDER)/%/ | $(APP_OUT)
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
