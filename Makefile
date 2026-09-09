# Configuration

# BUILD_ITEMS only available for the European release.

CHECKSUM       ?= 1
NON_MATCHING   ?= 0
SKIP_ASM       ?= 0

# Enables usage of the `COMMON` segment.
# The compiler can create a segment call `COMMON` intended to handle repetitive
# global variables declared in files which later becomes simple data that then
# get assigned into some data segment. This comes as default and can be disabled,
# however, MASPSX by default do not emit the segment and instead assign data
# directly into the data segments. The importance of having this segment
# supported is specially in the way MASPSX handle data order in a different way
# to how the actual `COMMON` segment handles data order. It is required to have
# the segment supported in the linker script, even though Splat doesn't natively
# support it. Additionally, the linker will reorder variables based on their names.
USE_COMMON     ?= 0

# Fixes Objdiff build feature.
OBJDIFF_FIX    ?= 0

# Names and Paths

GAME_VERSION = JP1

ifeq ($(GAME_VERSION), JP1)
GAME_NAME        := SLPS-91029
GAME_VERSION_DIR := JP1
GAME_FILE_EXE    := SLPS_005.00
else ifeq ($(GAME_VERSION), US)
GAME_NAME        := SLUS-00339
GAME_VERSION_DIR := US
GAME_FILE_EXE    := SLUS_003.39
endif

ROM_DIR         := rom
CONFIG_BASE_DIR := configs
CONFIG_DIR      := $(CONFIG_BASE_DIR)/$(GAME_VERSION_DIR)
LINKER_BASE_DIR := linkers
LINKER_DIR      := $(LINKER_BASE_DIR)/$(GAME_VERSION_DIR)
IMAGE_DIR       := $(ROM_DIR)/image
BUILD_BASE_DIR  := build
BUILD_DIR       := $(BUILD_BASE_DIR)/$(GAME_VERSION_DIR)
NM_BUILD_BASE_DIR := build/nm
NM_BUILD_DIR      := $(NM_BUILD_BASE_DIR)/$(GAME_VERSION_DIR)
OUT_DIR         := $(BUILD_DIR)/out
TOOLS_DIR       := tools
OBJDIFF_DIR     := $(TOOLS_DIR)/objdiff
PERMUTER_DIR    := permuter
ASSETS_DIR      := assets
ASM_BASE_DIR    := asm
ASM_DIR         := $(ASM_BASE_DIR)/$(GAME_VERSION_DIR)
C_DIR           := src
EXPECTED_DIR    := expected

# Tools
# Note: CPP stands for `C Pre-Processor` not `C++`

CROSS   := mipsel-linux-gnu
AS      := $(CROSS)-as
LD      := $(CROSS)-ld
OBJCOPY := $(CROSS)-objcopy
OBJDUMP := $(CROSS)-objdump
CPP     := $(CROSS)-cpp
CC      := $(TOOLS_DIR)/bin/gcc-2.6.0/cc1
OBJDIFF := $(OBJDIFF_DIR)/objdiff

PYTHON          := .venv/bin/python3
SPLAT           := $(PYTHON) -m splat split
MASPSX          := $(PYTHON) $(TOOLS_DIR)/maspsx/maspsx.py
DUMPSXISO       := $(TOOLS_DIR)/psxiso/dumpsxiso
MKPSXISO        := $(TOOLS_DIR)/psxiso/mkpsxiso
GET_YAML_TARGET := $(PYTHON) $(TOOLS_DIR)/get_yaml_target.py
GET_O_FILES     := $(PYTHON) $(TOOLS_DIR)/get_o_files.py
COMPTEST        := $(TOOLS_DIR)/compilationTest.sh

# Flags
OPT_FLAGS           := -O2
DL_FLAGS            := -G0
ENDIAN              := -EL
INCLUDE_FLAGS       := -Iinclude -I $(BUILD_DIR) -Iinclude/psyq -Iinclude/decomp
# LANGUAGE_C without the leading underscore is what Psy-Q 3.5's asm.h tests: it
# only skips the assembler's register aliases (v0, a0, fp ...) when that is
# defined, and those otherwise rewrite the parameter names of every prototype
# declared after kernel.h.
DEFINE_FLAGS        := -DLANGUAGE_C -D_LANGUAGE_C -DUSE_INCLUDE_ASM
CPP_FLAGS           := $(INCLUDE_FLAGS) $(DEFINE_FLAGS) -P -MMD -MP -undef -Wall -lang-c -nostdinc -DVER_${GAME_VERSION}
ifeq ($(USE_COMMON),1)
LD_FLAGS_GCSECTIONS := --no-gc-sections
COMMON_FLAG         := --use-comm-section
else
LD_FLAGS_GCSECTIONS :=
COMMON_FLAG         :=
endif
LD_FLAGS            := $(ENDIAN) $(DL_FLAGS) $(OPT_FLAGS) $(LD_FLAGS_GCSECTIONS) -nostdlib --no-check-sections
OBJCOPY_FLAGS       := -O binary
OBJDUMP_FLAGS       := --disassemble-all --reloc --disassemble-zeroes -Mreg-names=32
ifeq ($(GEN_COMP_TU),1)
SPLAT_FLAGS         := --disassemble-all --make-full-disasm-for-code
else
SPLAT_FLAGS         := --disassemble-all
endif
DUMPSXISO_FLAGS     := -x "$(ROM_DIR)/$(GAME_VERSION)" -s "$(ROM_DIR)/$(GAME_VERSION)/layout.xml" "$(IMAGE_DIR)/$(GAME_NAME).bin"
MKPSXISO_FLAGS      := -y -q "$(ROM_DIR)/$(GAME_VERSION)/rebuild.xml"

# Adjusts compiler and assembler flags based on source file location.
# - Files under main executable paths use -G8; overlay files use -G0.
define FlagsSwitch
    $(if $(or $(findstring /main/,$(1)),$(findstring /atlus/,$(1)),$(findstring /open/,$(1))),$(eval DL_FLAGS = -G8),$(eval DL_FLAGS = -G0))

	$(if $(or $(findstring /atlus/,$(1)),$(findstring /open/,$(1))),$(eval OPT = -O0),$(eval OPT = $(OPT_FLAGS)))

	$(eval LD_FLAGS = $(ENDIAN) $(DL_FLAGS) $(OPT_FLAGS) $(LD_FLAGS_GCSECTIONS) -nostdlib --no-check-sections)
	$(eval AS_FLAGS = $(ENDIAN) $(INCLUDE_FLAGS) $(OPT_FLAGS) $(DL_FLAGS) -march=r3000 -mtune=r3000 -no-pad-sections)
	$(eval CC_FLAGS = $(OPT) $(DL_FLAGS) -mips1 -mcpu=3000 -w -funsigned-char -fpeephole -ffunction-cse -fpcc-struct-return -fcommon -fverbose-asm -msoft-float -mgas -fgnu-linker -fdollars-in-identifiers -quiet)
	$(eval ASPSX_VERSION := 2.34)
	$(eval MASPSX_FLAGS = --gnu-as-path $(AS) --aspsx-version=$(ASPSX_VERSION) $(COMMON_FLAG) --run-assembler --expand-div $(AS_FLAGS))
	# src/common is compiled into several overlays, and their work areas do not
	# sit at the same address. Those sources reach a hardcoded address as
	# base + WORK_BIAS, so each overlay has to say how far its own area is
	# above the one the others share.
	$(if $(findstring /s2d/,$(1)),$(eval OVL_FLAGS := -DWORK_BIAS=0x20000),$(eval OVL_FLAGS := -DWORK_BIAS=0))
endef

ifeq ($(NON_MATCHING),1)
	CPP_FLAGS := $(CPP_FLAGS) -DNON_MATCHING
endif

ifeq ($(SKIP_ASM),1)
	CPP_FLAGS := $(CPP_FLAGS) -DSKIP_ASM
endif

# Utils

# Function to find matching .bin files for a target name.
find_bin_files = $(shell find $(ASM_DIR)/$(strip $1) -type f -path "*.bin" 2> /dev/null)

# Function to find matching .s files for a target name.
find_s_files = $(shell find $(ASM_DIR)/$(strip $1) -type f -path "*.s" -not -path "asm/*matchings*" 2> /dev/null)

# Function to find matching .c files for a target name.
find_c_files = $(shell find $(C_DIR)/$(strip $1) -type f -path "*.c" 2> /dev/null)

get_o_files = $(shell $(GET_O_FILES) $1 $(GAME_VERSION_DIR) $2 $(BUILD_BASE_DIR))

# Function to get path to .yaml file for given target.
get_yaml_path = $(addsuffix .yaml,$(addprefix $(CONFIG_DIR)/,$1))

# Function to get target output path for given target.
get_target_out_slow = $(patsubst $(OUT_DIR)/$(GAME_VERSION)%,$(OUT_DIR)%,$(addprefix $(OUT_DIR)/,$(shell $(GET_YAML_TARGET) $(call get_yaml_path,$1))))

# Faster version of the above using a cache lookup.
get_target_out = $(patsubst $(strip $1)|%,%,$(filter $(strip $1)|%,$(TARGET_OUT_CACHE)))

# Template definition for elf target.
# First parameter should be source target with folder (e.g. screens/credits).
# Second parameter should be end target (e.g. build/VIN/STF_ROLL.BIN).
# If we skip the ASM inclusion to determine progress, we will not be able to link. Skip linking, if so.

ifeq ($(SKIP_ASM),1)

define make_elf_target
$2: $2.elf

$2.elf: $(call get_o_files, $1, $(GEN_COMP_TU))
#endef make_elf_target
endef

#else SKIP_ASM
else

define make_elf_target

$2: $2.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS) $$< $$@

$2.elf: $(call get_o_files, $1, $(GEN_COMP_TU)) \
		$(LINKER_DIR)/$1.ld \
		$(LINKER_DIR)/$(filter-out ./,$(dir $1))undefined_syms_auto.$(notdir $1).txt \
		$(LINKER_DIR)/$(filter-out ./,$(dir $1))undefined_funcs_auto.$(notdir $1).txt \
		$(CONFIG_DIR)/externs.$1.ld
	@mkdir -p $(dir $2)
	$(LD) $(LD_FLAGS) \
		-Map $2.map \
		-T $(LINKER_DIR)/$1.ld \
		-T $(LINKER_DIR)/$(filter-out ./,$(dir $1))undefined_syms_auto.$(notdir $1).txt \
		-T $(LINKER_DIR)/$(filter-out ./,$(dir $1))undefined_funcs_auto.$(notdir $1).txt \
		-T $(CONFIG_DIR)/externs.$1.ld \
		-o $$@

#endef make_elf_target
endef

#endif SKIP_ASM
endif

# Targets

TARGET_IN := main atlus open movie end dng btlp s2d adv casino name
ifeq ($(GAME_VERSION), US)
TARGET_IN := $(TARGET_IN) nld
endif

# Source Definitions

ifneq ($(OBJDIFF_FIX),1)
TARGET_OUT_CACHE := $(foreach target,$(TARGET_IN),$(target)|$(call get_target_out_slow,$(target)))
endif

TARGET_OUT := $(foreach target,$(TARGET_IN),$(call get_target_out,$(target)))

CONFIG_FILES := $(foreach target,$(TARGET_IN),$(call get_yaml_path,$(target)))
LD_FILES     := $(addsuffix .ld,$(addprefix $(LINKER_DIR)/,$(TARGET_IN)))

# Recursively include any .d dependency files from previous builds.
# Allowing Make to rebuild targets when any included headers/sources change.
-include $(shell [ -d $(BUILD_DIR) ] && find $(BUILD_DIR) -name '*.d' || true)

# Rules

# Rules - Build (Normal/Matching)

default: build

build: $(TARGET_OUT) checksum

reset-build:
	rm -rf $(BUILD_DIR)/src
	rm -rf $(OUT_DIR)
	$(MAKE) build $(TARGET_OUT) checksum

clean-build: reset
	$(MAKE) generate
	$(MAKE) build

generate: $(LD_FILES)

# Rules - Build (Objdiff/Progress)

objdiff-config:
	rm -rf $(EXPECTED_DIR)
	mkdir -p $(EXPECTED_DIR)
	$(MAKE) GAME_VERSION=$(GAME_VERSION) objdiff-generate
	$(PYTHON) $(OBJDIFF_DIR)/objdiff_generate.py $(OBJDIFF_DIR)/config-retail.yaml 	        $(GAME_VERSION_DIR) --build-base $(NM_BUILD_BASE_DIR)

objdiff-config-all:
	rm -rf $(EXPECTED_DIR)
	mkdir -p $(EXPECTED_DIR)
	$(MAKE) GAME_VERSION=JP1 objdiff-generate
#	$(MAKE) GAME_VERSION=US objdiff-generate
	$(PYTHON) $(OBJDIFF_DIR)/objdiff_generate.py $(OBJDIFF_DIR)/config-retail.yaml ALL 	        --build-base $(NM_BUILD_BASE_DIR)

objdiff-generate:
	$(MAKE) GAME_VERSION=$(GAME_VERSION) GEN_COMP_TU=1 regenerate
	rm -rf $(NM_BUILD_DIR)
	$(MAKE) GAME_VERSION=$(GAME_VERSION) NON_MATCHING=1 SKIP_ASM=1 GEN_COMP_TU=1 	        BUILD_BASE_DIR=$(NM_BUILD_BASE_DIR) build
	mv $(NM_BUILD_DIR)/$(ASM_DIR) $(EXPECTED_DIR)

report:
	$(MAKE) GAME_VERSION=$(GAME_VERSION) objdiff-config
	$(OBJDIFF) report generate > $(BUILD_DIR)/progress.json

progress:
	$(MAKE) NON_MATCHING=1 SKIP_ASM=1 BUILD_BASE_DIR=$(NM_BUILD_BASE_DIR) build

reset-progress:
	rm -rf $(NM_BUILD_DIR)
	$(MAKE) NON_MATCHING=1 SKIP_ASM=1 BUILD_BASE_DIR=$(NM_BUILD_BASE_DIR) build

# Rules - Rom handling (Extraction/Insertion)
# TODO: Allow to insert any files to the game files
# At the moment only overlays have been handled

setup: reset-root
	rm -rf $(EXPECTED_DIR)
	$(MAKE) extract
	$(MAKE) generate

extract:
	$(DUMPSXISO) $(DUMPSXISO_FLAGS)

# Rules - Cleaning

clean-rom:
	find $(ROM_DIR) -maxdepth 1 -type f -delete
	find $(ROM_DIR)/$(GAME_VERSION_DIR) -maxdepth 1 -type f -delete

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(NM_BUILD_DIR)
	rm -rf $(PERMUTER_DIR)

reset: clean
	rm -rf $(ASM_DIR)
	rm -rf $(LINKER_DIR)

clean-root:
	rm -rf $(BUILD_DIR)
	rm -rf $(PERMUTER_DIR)

reset-root: clean-root
	rm -rf $(ASM_DIR)
	rm -rf $(LINKER_DIR)

# Rules - Misc.

regenerate: reset
	$(MAKE) generate

compilation-test:
	$(COMPTEST)

checksum: $(TARGET_OUT)
ifeq ($(CHECKSUM),1)
ifeq ($(SKIP_ASM),0)
	@sha256sum --ignore-missing --check "$(CONFIG_DIR)/checksum.sha"
endif
endif

config-formatter:
	$(PYTHON) $(TOOLS_DIR)/configs_formatter.py

# Recipes

ifeq ($(OBJDIFF_FIX),1)
$2: $(info $(patsubst $(BUILD_DIR)/%.c.o,%.c,$2))
else
# .elf targets
# Generate .elf target for each target from TARGET_IN.
$(foreach target,$(TARGET_IN),$(eval $(call make_elf_target,$(target),$(call get_target_out,$(target)))))
endif

# Generate objects.
# (Running make with MAKE_COMPILE_LOG=1 will create a compile.log that can be passed to tools/create_compile_commands.py)

$(BUILD_DIR)/%.i: %.c
	@mkdir -p $(dir $@)
	$(call FlagsSwitch, $@)
ifeq ($(MAKE_COMPILE_LOG),1)
	@echo "$(CPP) -P -MMD -MP -MT $@ -MF $@.d $(CPP_FLAGS) $(OVL_FLAGS) -o $@ $<" >> compile.log
endif
	$(CPP) -P -MMD -MP -MT $@ -MF $@.d $(CPP_FLAGS) $(OVL_FLAGS) -o $@ $<

$(BUILD_DIR)/%.sjis.i: $(BUILD_DIR)/%.i
	$(PYTHON) $(TOOLS_DIR)/iconv_sjis_wrapper.py -f UTF-8 -t SHIFT-JIS $< -o $@

$(BUILD_DIR)/%.c.s: $(BUILD_DIR)/%.sjis.i
	@mkdir -p $(dir $@)
	$(call FlagsSwitch, $@)
	$(CC) $(CC_FLAGS) -o $@ $<

$(BUILD_DIR)/%.c.o: $(BUILD_DIR)/%.c.s
	@mkdir -p $(dir $@)
	$(call FlagsSwitch, $@)
	$(MASPSX) $(MASPSX_FLAGS) -o $@ $< < /dev/null

ifneq ($(OBJDIFF_FIX),1)

$(BUILD_DIR)/%.s.o: %.s
	@mkdir -p $(dir $@)
	$(call FlagsSwitch, $@)
	$(AS) $(AS_FLAGS) -o $@ $<

$(BUILD_DIR)/%.bin.o: %.bin
	@mkdir -p $(dir $@)
	$(LD) $(LD_FLAGS) -r -b binary -o $@ $<

# Split .yaml.
$(LINKER_DIR)/%.ld: $(CONFIG_DIR)/%.yaml
	@mkdir -p $(dir $@)
	$(SPLAT) $(SPLAT_FLAGS) $<

endif

### Settings
.SECONDARY:
.PHONY: clean default build
SHELL = /bin/bash -e -o pipefail
