# Persona 1 (JP) — matching decompilation

Matching decomp of *Megami Ibunroku Persona* (SLPS-00500, PlayStation). JP is the
base. Persona 2 (Innocent Sin / Eternal Punishment) is a later goal and shares
the engine, which is why the PSX release was chosen over the 1999 JP PC port —
the port flattens the overlay set, duplicates functions per overlay, and shares
almost nothing with P2.

Read `README.md` for the full picture; this file is the operating manual.

## Environment

Builds run in **WSL (Ubuntu)** against the repo on `/mnt/g/projects/persona-1-2-2`.
Windows-side tools (Ghidra via ReVa MCP) see the same path as `G:\projects\...`.

    wsl -e bash -lc 'cd /mnt/g/projects/persona-1-2-2 && ...'

Always export `PYTHONDONTWRITEBYTECODE=1` and `rm -rf tools/__pycache__` after
editing anything in `tools/`. The 9p mount gives Windows and WSL different
mtimes, so Python happily imports a stale `.pyc` and you will debug a change that
was never loaded.

Python lives in `.venv`. Disc images and extracted game data are in `scratch/`
(gitignored, ~9 GB).

## Commands

    make split      # gen_symbols from the Ghidra map, then splat every target
    make            # assemble + link all 11 targets
    make check      # byte-compare every target against the disc original
    make progress   # re-verify every decompiled function

    tools/mfunc.py <target> <symbol> [cand.c] [--diff] [--all]
    tools/pick_candidates.py [per] [min_insn] [max_insn]
    tools/find_dups.py [minbytes] | <target> <symbol>
    tools/setup_permuter.py <target> <symbol> <cand.c>

Targets: `main atlus open movie end dng btlp s2d adv casino name`.

## Invariants

- **`make check` must stay byte-exact for all 11 targets.** It is the ground
  truth that the split is lossless. Never let it regress.
- **Only a 100% objdiff counts as matched.** `tools/progress.py` enforces this;
  a decompiled-but-unmatched function is listed as in-progress.
- **No address-derived names in `src/`.** No `func_8007F2B0`, no `D_8009FB28`.
  If you cannot justify a name, investigate until you can — do not invent one
  that sounds confident, and do not leave the address form in place.
- **Never decompile the Psy-Q SDK.** It is already in the binaries and will be
  linked from the real libraries. `tools/pick_candidates.py` filters anything
  Ghidra has named for exactly this reason. (`tools/find_dups.py` is finer: it
  filters only names the PsyQ signature analyser applied, so functions *we*
  named still show up — those are the ones worth finding twins for.)
- **Check for twins before writing a source.** The overlays and sub-EXEs each
  compile in their own copy of a shared routine, so one source often covers
  several targets. `tools/find_dups.py <target> <symbol>` says which; shared
  sources live in `src/p1-jp/common/` with target-neutral names.

## Naming

Ghidra is the source of truth, and the only copy. Rename by running a pyghidra
script through ReVa `run-script` against the program that owns the address —
every overlay loads at `0x800643A0` and every sub-EXE at `0x80080000`, so one
address means different things in different binaries. Then:

1. run `ghidra/ExportCodeMap.py` in Ghidra
2. `tools/gen_names.py`
3. update the affected `src/` files **by hand**
4. `tools/progress.py` to confirm nothing regressed

Renaming an already-named symbol will not propagate itself; step 3 is manual and
skipping it silently desynchronises the source from the target.

Record the evidence for a name as a **plate comment on the symbol**, set in the
same script. It is not decoration: every asserted name should be traceable to
something observed — an SDK call it wraps, a constant it writes, a sentinel
value, a caller's behaviour.

`config/p1-jp/rename.txt` and `ghidra/ApplyNames.py` are the retired version of
this flow. Do not add rows to it.

`ghidra/FindHiLoRefs.py` answers "who touches this address" when Ghidra's own
xrefs come up empty. Ghidra resolves `lui`/`%lo` pairs only inside `ram` and
drops the `lui at,%hi(X) / addu at,at,idx / lw r,%lo(X)(at)` indexing idiom
entirely, so array members and overlay-side readers look unreferenced.

## Comments in src/

Comment the **game**: what a structure holds, what a routine does, what a
constant means, which file on the disc the data came from. That is what the
sources are for, and it is the part nobody can reconstruct later.

Do not write up the matching process. Which spelling of a loop gcc reproduced,
what the permuter found, what percentage the wrong shape scored - none of that
belongs beside the code. It is scaffolding, it stops being true the moment the
function is matched, and it buries the few lines that say what the code does.

The one exception is a spelling that looks redundant and would get "tidied"
back: leave a single sentence saying not to, not a paragraph explaining gcc.
General compiler behaviour goes in the gotchas section below, once.

## Editing

Use the Edit/Write tools for file changes. Patching files with Python embedded in
a shell heredoc has repeatedly mangled escapes here — it once wrote a literal
backspace byte into a regex, which `grep` cannot display and which cost a long
debugging detour.

## Gotchas that have already cost time

- `cc1` is the compiler proper and does **not** preprocess. Run `cpp` first.
- `maspsx` reads assembly on **stdin**; it takes no positional file argument.
- Compare **objects**, not linked bytes. `ld` aligns `.text` to 16, so a function
  at a non-16-aligned address shifts and every absolute `j` resolves wrong.
- Target and candidate must agree on **symbol names** or identical code scores
  below 100%. `normalize_asm` in `tools/mfunc.py` handles the known cases.
- Not every translation unit uses the same flags. `ATLUS.EXE`, `OPEN.EXE` and
  `main` itself are built with a small-data area; use `/* cc1flags: -O2 -G8 */`
  (`-O0` for the sub-EXEs, which also keep a frame pointer) and record the
  binary's `$gp` in `config/p1-jp/gp.txt` (it is set at the entry point, not in
  the PS-EXE header).

### Literal address or linker symbol?

The game reaches most of its work area (`0x801Dxxxx`-`0x801Fxxxx`,
`0x800Dxxxx`-`0x800Fxxxx`) by **hardcoded address**, not through a symbol. Which
one the source used is visible in the encoding, and getting it wrong caps a
function well below 100%:

- An indexed access through a symbol assembles to `addu $at,$at,rX`; through a
  literal to `addu $at,rX,$at`. `normalize_asm` folds the literal form back,
  keyed on the address so naming the symbol later cannot break it.
- A symbol is **rematerialised** (`lui`/`%hi` + `lw`/`%lo`) at every access. A
  literal gets CSE'd into a register once, so a stray `la`/`ori` base register in
  your output means you wrote a literal where the original had a symbol.
- spimdisasm symbolises *some* `lui`/load pairs and leaves others raw. If a
  function builds an address explicitly (splat writes `(0xADDR >> 16)`), the
  invented symbols nearby are literals too - `normalize_asm` folds those.
- Under `-G8`, a plain `extern int x;` becomes `%gp_rel`. For a work-area global
  that is wrong; declare it as an **incomplete array** (`extern int x[];`) so gcc
  cannot assume it is small enough for `.sdata`.
- Indexing a literal base (`((T *)0xADDR)[i].f`) folds the constant into each
  access. If the original keeps one base register with small offsets, use a
  pointer local instead: `T *p = (T *)0xADDR;`.

### Store order

gcc hoists constant stores. If a run of constant assignments comes out earlier
than the original has it, move those statements to the **end** of the function
and let gcc hoist them into place - writing them in their apparent position is
what puts them too early. This is what took `SlotInit` from 70% to 100%.

### Bit setters with a shared store tail

A function that sets or clears one bit usually compiles to *two* loads and
*one* store, both arms jumping to a common tail. Writing that as

    if (on) k = p->attr | BIT; else k = p->attr & ~BIT;
    p->attr = k;

gets the tail right but recomputes the scaled index in all three basic blocks,
because gcc 2.6 has no global CSE. Two things together fix it:

- reach the record through a **pointer local** (`Slot *s = g_slots + slot;`).
  That forces the index to be scaled once, ahead of the branch - even though
  gcc then goes back to `lui/%hi + addu` addressing for each access and never
  materialises the pointer.
- write the update as a **compound assignment in both arms**
  (`s->attr |= BIT;` / `s->attr &= ~BIT;`). gcc's cross-jumping merges the two
  identical stores into the shared tail by itself.

Either alone lands around 40-65%; together they are exact. This took
`SlotSetFlicker`, `SlotSetSemiTrans` and the two per-slot fades to 100%.

### Declare the real SDK struct

If a table turns out to be an array of a Psy-Q type, declare the genuine type
rather than a placeholder with the right size. `include/psyq/libgs.h` shipped
here as a three-function stub, and a hand-rolled 0x24-byte struct with the
cleared bytes at offsets 0..2 left `FadeBlackout` stuck at 99.7%: gcc biased
the induction variable to the *first* field. Writing `GsSPRITE` properly -
where those bytes are `r`, `g`, `b` at 0x14..0x16 of the record - put the base
where the original has it and the function matched. Field offsets inside the
struct decide induction-variable placement, so the layout has to be right,
not merely the size.

## Layout

    bin/              downloaded toolchain (gitignored; see bin/*.sha256)
    config/p1-jp/     splat configs, symbol maps, rename.txt, gp.txt, ghidra/*.json
    docs/             memory map and program structure
    ghidra/           scripts run inside Ghidra via ReVa
    include/psyq/     Psy-Q SDK headers
    include/persona/  our own headers, mirroring src/ (<target>/<name>.h)
    src/p1-jp/        decompiled C, one directory per target, plus common/
                      for sources that match in more than one target
    tools/            the pipeline, plus vendored maspsx / m2c / asm-differ / permuter

### Keeping the tree navigable

Sources are grouped by subsystem once a directory outgrows a single listing -
roughly ten files. The groups in use are `gfx/`, `ui/`, `game/` and `audio/`.

The important rule is that **a source and its per-target copies live at the same
relative path**: `common/gfx/slot.c` and `s2d/gfx/slot.c`, `common/ui/input.c`
and `s2d/ui/input.c`. Those pairs exist only because one overlay reaches a
different address, so they have to be easy to spot and to diff. Introducing a
subdirectory in `common/` therefore means creating it in every target directory
that overrides one of the files in it.

Nothing in the build walks `src/` - the Makefile compiles from `asm/`, and
candidates are verified standalone through `tools/mfunc.py`. Moving a source is
`git mv` plus the path in `config/p1-jp/decomp.txt`, and `tools/progress.py` is
the check that you got every row. Do the move as its own commit; mixing it with
a match makes both harder to read.

### Switch tables

A `switch` with more than a handful of dense cases compiles to a bounds test and
a jump through a table of case addresses. splat leaves that table sitting in the
overlay's data run, where it is reached through whichever symbol happens to
precede it, while a candidate emits its own into `.rodata` - so the dispatch
relocation could never agree and every such function capped just below 100%.

`rebuild_jump_tables` in `tools/mfunc.py` re-emits the original table as a local
`.rodata` label on the target side, padded to sit at the same offset the
candidate's does, and `tables_agree` compares the two tables afterwards. The
case addresses still come from the original, so a candidate whose cases are in
the wrong order is reported as `mismatch - the switch table differs` rather than
passing on identical code. `tools/pick_candidates.py` still filters these out;
they are matchable now, so that filter is conservative rather than correct.

### Reading the game's text

`tools/glyphs.py` decodes the packed bytes `TileMapWriteRow` expands into
character-map cells. The font puts A-Z at 0xA6..0xBF and 0-9 at 0xC0..0xC9, so
the Latin parts of the data read straight out:

    tools/glyphs.py adv 800B18A8 12    # one label
    tools/glyphs.py adv --scan         # every label in the target

Most of the text is Japanese and outside that range, but the tables that are
not - the arcana names, the elements, the status ailments, the name-entry
keyboard - are enough to identify the tables around them, and a string constant
with a readable name beats one called after its address.
