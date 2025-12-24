
ROOT := ..
USER_FOLDER := $(ROOT)/user
ARCH        ?= riscv                        # o x86


APPS_FOLDER := $(USER_FOLDER)/apps
SERVERS_FOLDER := $(USER_FOLDER)/servers
BUILD_FOLDER := $(USER_FOLDER)/build

# ----------------------------
# App discovery
# ----------------------------

# 4. servers multi-file (folders) apps
SERVERS_DIRS := $(filter-out $(SERVERS_FOLDER)/build/, $(wildcard $(SERVERS_FOLDER)/*/))

# 5. general single-file (.c)
GENERAL_C := $(wildcard $(APPS_FOLDER)/*.c)

# 6. general multi-file (folders)
GENERAL_DIRS := $(filter-out $(APPS_FOLDER)/build/, $(wildcard $(APPS_FOLDER)/*/))


GENERAL_C_NAMES       := $(basename $(notdir $(GENERAL_C)))
GENERAL_DIR_NAMES     := $(notdir $(patsubst %/,%,$(GENERAL_DIRS)))

SERVERS_DIR_NAMES     := $(notdir $(patsubst %/,%,$(SERVERS_DIRS)))


META_APP_NAMES := \
	$(SERVERS_DIR_NAMES) \
	$(GENERAL_C_NAMES) \
	$(GENERAL_DIR_NAMES)


# Append main apps i.e shell
META_APP_NAMES += shell
META_APP_NAMES += tests_shell

META_APP_NAMES     := $(sort $(META_APP_NAMES))

DEF_EXECUTABLES:= touch rm stat

### Ensure def excecutables are at the start
META_APP_NAMES := $(filter-out $(DEF_EXECUTABLES),$(META_APP_NAMES))

META_APP_NAMES := $(strip \
	$(DEF_EXECUTABLES) \
	$(META_APP_NAMES))


DEF_EXECUTABLE_COUNT := $(words $(DEF_EXECUTABLES))

.PHONY: servers_dirs general_c general_dirs all


all: metadata servers_dirs general_c general_dirs
	@echo "NOW COMPILE MAIN APPS ... i.e shell app"
# 	@./compile_dir_app.sh shell
# 	@./compile_dir_app.sh tests_shell

##
## Metadata building
##
metadata:
# 	@echo "Generate meta got apps $(META_APP_NAMES) ... DEF exec count $(DEF_EXECUTABLE_COUNT)"
	@./generate_apps_meta.sh $(DEF_EXECUTABLE_COUNT) $(META_APP_NAMES)
clean:
	@rm -r $(BUILD_FOLDER)
	@mkdir -p $(BUILD_FOLDER)

servers_dirs:
	@echo "Building servers multi-file apps:"
	@for d in $(SERVERS_DIRS); do \
		echo "  [SRV_DIR] $$d"; \
	done
general_c:
	@echo "Building general single-file apps:"
	@for f in $(GENERAL_C); do \
		echo "  [GEN_C] $$f"; \
	done
general_dirs:
	@echo "Building general multi-file apps:"
	@for d in $(GENERAL_DIRS); do \
		echo "  [GEN_DIR] $$d"; \
	done
