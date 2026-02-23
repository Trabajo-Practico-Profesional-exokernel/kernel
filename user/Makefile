ROOT := ..
USER_FOLDER := $(ROOT)/user
ARCH ?= riscv

APPS_FOLDER     := $(USER_FOLDER)/apps
DIR_APPS_FOLDER := $(USER_FOLDER)/dir_apps
SERVERS_FOLDER  := $(USER_FOLDER)/servers
BUILD_FOLDER    := $(USER_FOLDER)/build
APP_OUT         := $(BUILD_FOLDER)

TRG_DISK       ?= $(ROOT)/.kernel_disk/disk.img
TRG_BASE_DISK  ?= $(ROOT)/.kernel_disk/disk_base.img
TRG_DISK_SIZE  := $(shell echo $$((20 * 1024 * 1024)))


# -------- Single-file apps --------
GENERAL_C := $(shell find $(APPS_FOLDER) -type f -name '*.c')
GENERAL_REL := $(patsubst $(APPS_FOLDER)/%,%,$(GENERAL_C))
GENERAL_NAMES := $(GENERAL_REL:.c=)
GENERAL_BINS := $(addprefix $(APP_OUT)/,$(GENERAL_NAMES))

# -------- Directory apps --------
GENERAL_DIRS := $(shell find $(DIR_APPS_FOLDER) -mindepth 1 -maxdepth 1 -type d)
GENERAL_DIR_NAMES := $(notdir $(GENERAL_DIRS))
GENERAL_DIR_BINS := $(addprefix $(APP_OUT)/,$(GENERAL_DIR_NAMES))

# -------- Servers --------
SERVER_DIRS := $(shell find $(SERVERS_FOLDER) -mindepth 1 -maxdepth 1 -type d)
SERVER_NAMES := $(notdir $(SERVER_DIRS))
SERVER_BINS := $(addprefix $(APP_OUT)/,$(SERVER_NAMES))

MAIN_APP_NAMES := shell

META_APP_NAMES := \
	$(SERVER_NAMES) \
	$(GENERAL_NAMES) \
	$(GENERAL_DIR_NAMES) \
	$(MAIN_APP_NAMES)

META_APP_NAMES := $(sort $(META_APP_NAMES))

DEF_EXECUTABLES := touch rm stat cat

META_APP_NAMES := $(filter-out $(DEF_EXECUTABLES),$(META_APP_NAMES))
META_APP_NAMES := $(strip $(DEF_EXECUTABLES) $(META_APP_NAMES))

DEF_EXECUTABLE_COUNT := $(words $(DEF_EXECUTABLES))


ALL_BINS := \
	$(GENERAL_BINS) \
	$(GENERAL_DIR_BINS) \
	$(SERVER_BINS)

.PHONY: all clean metadata disk_image apps app shell

all: $(APP_OUT) metadata $(ALL_BINS)
	@echo "Building main apps"
	@./compile_dir_apps.sh $(ARCH) -add_meta $(MAIN_APP_NAMES)
	@$(MAKE) disk_image

disk_image:
	@python3 disk_image_builder.py $(BUILD_FOLDER) $(TRG_BASE_DISK) $(TRG_DISK_SIZE)
	cp $(TRG_BASE_DISK) $(TRG_DISK)

apps:
	@echo $(META_APP_NAMES)

app:
	@if [ -z "$(APP)" ]; then \
		echo "Usage: make app APP=name"; \
		exit 1; \
	fi
	@$(MAKE) $(APP_OUT)/$(APP)
	@$(MAKE) disk_image

shell:
	@./compile_dir_apps.sh $(ARCH) -add_meta shell
	@$(MAKE) disk_image

clean:
	rm -rf $(BUILD_FOLDER)
	

$(GENERAL_BINS): $(APP_OUT)/%: $(APPS_FOLDER)/%.c | $(APP_OUT)
	@mkdir -p $(dir $@)
	@echo "Compiling single-file app: $*"
	@./compile_file_apps.sh $(ARCH) $(APPS_FOLDER) $*

$(GENERAL_DIR_BINS): $(APP_OUT)/%: 
	@echo "Compiling dir app: $*"
	@./compile_dir_apps.sh $(ARCH) $(DIR_APPS_FOLDER)/$*

$(SERVER_BINS): $(APP_OUT)/%:
	@echo "Compiling server: $*"
	@./compile_dir_apps.sh $(ARCH) -add_meta $(SERVERS_FOLDER)/$*

$(APP_OUT):
	@mkdir -p $(APP_OUT)

