# Persona 1 (JP) - matching decompilation build
#
# Every target is currently 100% split asm, so `make check` must reproduce each
# original binary byte-for-byte. That is the toolchain's ground-truth test.

GAME      := p1-jp
DATA      := scratch/extracted/$(GAME)

CROSS     := mipsel-linux-gnu-
AS        := $(CROSS)as
LD        := $(CROSS)ld
OBJCOPY   := $(CROSS)objcopy
CC1       := bin/cc1-psx-26/cc1-psx-26
# cc1 is the compiler proper and does not preprocess; cpp must run first.
CPP       := $(CROSS)gcc -E
CPPFLAGS  := -nostdinc -undef -D__GNUC__=2 -Iinclude -Iinclude/psyq
MASPSX    := .venv/bin/python3 tools/maspsx/maspsx.py
PYTHON    := .venv/bin/python3

ASPSX_VER := 2.34
# Code/data boundaries come from Ghidra (tools/gen_symbols.py), so split asm
# assembles at real R3000 with no permissive-arch fallback.
ASFLAGS   := -EL -march=r3000 -mtune=r3000 -no-pad-sections -Iinclude
ASFLAGS_C := $(ASFLAGS)
CC1FLAGS  := -quiet -O2 -G0 -mcpu=3000 -msoft-float -mgas -gcoff -fgnu-linker \
             -funsigned-char -fpeephole -ffunction-cse -fpcc-struct-return \
             -fcommon -fverbose-asm -w
LDFLAGS   := --no-check-sections

# main + 4 Exec()'d sub-EXEs are PS-EXEs; the other 6 are raw overlay images.
PSEXE     := main atlus open movie end
OVERLAYS  := dng btlp s2d adv casino name
TARGETS   := $(PSEXE) $(OVERLAYS)

ORIG_main   := $(DATA)/SLPS_005.00
ORIG_atlus  := $(DATA)/EXE/ATLUS.EXE
ORIG_open   := $(DATA)/EXE/OPEN.EXE
ORIG_movie  := $(DATA)/EXE/MOVIE.EXE
ORIG_end    := $(DATA)/EXE/END.EXE
ORIG_dng    := $(DATA)/DNG.BIN
ORIG_btlp   := $(DATA)/BTLP.BIN
ORIG_s2d    := $(DATA)/S2D.BIN
ORIG_adv    := $(DATA)/ADV.BIN
ORIG_casino := $(DATA)/CASINO.BIN
ORIG_name   := $(DATA)/NAME.BIN

.PHONY: all check clean split symbols refs progress $(TARGETS)
all: $(TARGETS)

# ---- per-target build ------------------------------------------------------
# $(1) = target name
define TARGET_RULES

$(1)_ASM  := $$(shell find asm/$(GAME)/$(1) -name '*.s' 2>/dev/null)
# A decompiled function only enters the build once its splat subsegment is
# flipped from `asm` to `c` - otherwise it would exist twice (once from the
# split asm, once from C) and the byte-exact check would be meaningless.
# Until then, candidates in src/ are verified standalone via tools/mfunc.py.
$(1)_SRC  :=

$(1)_OBJ  := $$(patsubst %.s,build/$(GAME)/$(1)/%.s.o,$$($(1)_ASM)) \
             $$(patsubst %.c,build/$(GAME)/$(1)/%.c.o,$$($(1)_SRC))

$(1): build/$(GAME)/$(1).bin

build/$(GAME)/$(1)/%.s.o: %.s
	@mkdir -p $$(dir $$@)
	$$(AS) $$(ASFLAGS) -o $$@ $$<

build/$(GAME)/$(1)/%.c.o: %.c
	@mkdir -p $$(dir $$@)
	$$(CPP) $$(CPPFLAGS) $$< -o $$@.i
	$$(CC1) $$(CC1FLAGS) $$@.i -o $$@.s
	$$(MASPSX) --expand-div --aspsx-version=$$(ASPSX_VER) < $$@.s | $$(AS) $$(ASFLAGS_C) -o $$@ -

build/$(GAME)/$(1).elf: $$($(1)_OBJ) config/$(GAME)/$(1).ld
	@mkdir -p $$(dir $$@)
	$$(LD) $$(LDFLAGS) 	  -T config/$(GAME)/$(1).undefined_syms_auto.txt 	  -T config/$(GAME)/$(1).undefined_funcs_auto.txt 	  -T config/$(GAME)/$(1).undefined_extra.txt 	  -T config/$(GAME)/$(1).ld 	  -Map build/$(GAME)/$(1).map -o $$@

build/$(GAME)/$(1).bin: build/$(GAME)/$(1).elf
	$$(OBJCOPY) -O binary $$< $$@

endef
$(foreach t,$(TARGETS),$(eval $(call TARGET_RULES,$(t))))

# ---- verification ----------------------------------------------------------
check: all
	@$(PYTHON) tools/verify.py

progress:
	@$(PYTHON) tools/progress.py

refs:
	@$(PYTHON) tools/gen_refs.py

# Deliberately not `symbols: refs`. gen_refs reads the asm the last split
# produced, and a reference it harvests stops looking like `SYM + 0xNN` once
# the cut it asks for exists - so running it on every split makes the symbol
# map oscillate between two states rather than settle. It accumulates, and is
# run by hand when a new offset reference turns up.
symbols:
	@$(PYTHON) tools/gen_symbols.py

split: symbols
	@cd config/$(GAME) && for c in $(TARGETS); do ../../.venv/bin/splat split $$c.yaml >/dev/null || exit 1; done
	@echo "split complete"

clean:
	rm -rf build
