# Persona 1 (JP, SLPS-00500) — matching decompilation

Target: the PlayStation release, JP as the base. The 1999 JP PC port is kept as a
*reference oracle* only — it flattens the overlay set and duplicates functions per
overlay, and shares almost nothing with Persona 2, which is PSX-only.

## Status

All 11 targets rebuild **byte-identically** from split asm (`make check`), and
each has at least one function decompiled to matching C.

| target | binary | vram | asm rebuild |
|---|---|---|---|
| `main` | `SLPS_005.00` | `0x80010000` | MATCH |
| `atlus` `open` `movie` `end` | `EXE/*.EXE` | `0x80080000` | MATCH |
| `dng` `btlp` `s2d` `adv` `casino` `name` | `*.BIN` overlays | `0x800643a0` | MATCH |

6,347 functions are split out. `tools/progress.py` reports **11 matching,
1,892 / 1,536,604 bytes (0.123%)**; `--record` appends a timestamped row to
`progress.json`.

### Decompiled to C

| target | function | bytes |
|---|---|---|
| `main` | `LoadOverlay` | 212 |
| `dng` | `DngPollInput` | 172 |
| `s2d` | `S2dPollInput` | 180 |
| `adv` | `AdvPollInput` | 180 |
| `btlp` | `BtlWaitBgmEnd` | 168 |
| `casino` | `CasinoPlayStepAnim` | 184 |
| `name` | `NameUploadImageRows` | 164 |
| `atlus` | `StrWaitFrame` | 180 |
| `open` | `StrWaitFrame` | 180 |
| `movie` | `StrDecodeNextFrame` | 136 |
| `end` | `StrDecodeNextFrame` | 136 |

A decompiled function only joins the build once its splat subsegment is flipped
from `asm` to `c`; until then candidates live in `src/` and are verified
standalone, so `make check` stays a true asm-rebuild test.

## Toolchain

- `bin/cc1-psx-26` — Psy-Q cc1, self-identifies as `GNU C 2.6.3 [AL 1.1, MM 40] Sony Playstation`
- `tools/maspsx` — replicates ASPSX so GNU as can assemble Psy-Q compiler output
- `binutils-mipsel-linux-gnu` — assembler/linker (`elf32ltsmip`)
- `splat` + `spimdisasm` — splitting
- `include/psyq/` — Psy-Q SDK headers
- `bin/objdiff-cli`, `tools/decomp-permuter`, `tools/m2c`, `tools/asm-differ`

Everything runs natively on Linux; no Wine or DOSBox is needed.

## Build

    make split     # regenerate symbols from Ghidra, then split all targets
    make           # assemble + link all targets
    make check     # byte-compare every target against the disc original
    make progress  # re-verify every decompiled function

## Matching a function

    tools/mfunc.py <target> <symbol>                       # show the target
    tools/mfunc.py <target> <symbol> cand.c                # score a candidate
    tools/mfunc.py <target> <symbol> cand.c --diff         # ...with a diff
    tools/mfunc.py <target> <symbol> cand.c --diff --all   # include matching rows

It assembles the original function on its own, compiles the candidate, and hands
both objects to **objdiff** — which understands relocations and is what
decomp-permuter scores against, so its percentage is the verdict. There is no
hand-rolled linking or byte comparison in the loop: an earlier version had one,
and `ld` aligning `.text` to 16 silently *understated* every function at a
non-16-aligned address.

For the target and the candidate to be comparable they must agree on symbol
names, so `normalize_asm` rewrites the target asm three ways:

- spimdisasm's `D_8009FB00 + 0x28` offset form becomes `D_8009FB28`
- address-derived names become the real ones (`func_80012B2C` → `CdSearchFileLoc`)
- a `lui`/`addiu` pair building a large *constant* that spimdisasm mistook for an
  address (`%hi(D_7FFFFF)`) folds back to a literal
- a linked `0($gp)` small-data access becomes `%gp_rel(symbol)($gp)`, once the
  target's `$gp` is known (see `config/p1-jp/gp.txt`)

### Per-file compiler flags

Not every translation unit was built the same way. `ATLUS.EXE` and `OPEN.EXE` are
almost entirely SDK plus **`-O0` game code with a small-data area** — frame
pointers and `$gp`-relative accesses give it away. A source can override the
defaults:

    /* cc1flags: -O0 -G8 */

`$gp` itself is not in the PS-EXE header; each binary sets it at its entry point
(`ATLUS.EXE` does `$gp = 0x800A4DEC + 0x7AC`), and those values live in
`config/p1-jp/gp.txt`.

### When the structure is right but registers are not

    tools/setup_permuter.py <target> <symbol> <candidate.c>
    tools/decomp-permuter/permuter.py permuter/<target>/<symbol> -j12 --stop-on-zero

That is how `DngPollInput` landed: reusing one `int` for a call result and then
for `0x1F` puts the constant in a register before the pointer load. It took 873
iterations. `NameUploadImageRows` needed a similar trick — a redundant
`p = desc` to force the base into a register before the add.

Compiler behaviours worth knowing, each of which cost a match until fixed:

- **Values live across a call want a saved register.** `LoadOverlay` only matched
  once the sector count was a local computed before the retry loop.
- **Load/store order is visible.** MIPS1 needs a slot after a load; storing
  before loading lets gcc fill it with a store instead of the original's `nop`.
- **Return width shows up.** Two stray `andi v0, v0, 0xff` meant the callee
  returned `u_char`, not `int`.
- **Loop layout follows the source.** The FMV decoder only matched as an explicit
  `goto` chain; `do/while` and `for(;;)` both put the backward branch in the
  wrong place.
- **Struct access beats array indexing at `-O0`.** `header->size` emits
  `lw v1, 8(v0)`; `header[2]` computed the address first.

## Naming

Names live in Ghidra and flow outwards. After editing `config/p1-jp/rename.txt`:

1. run `ghidra/ApplyNames.py` in Ghidra — names the symbols
2. run `ghidra/ExportCodeMap.py` in Ghidra — re-exports the map
3. `tools/gen_names.py` — rebuilds the per-target linker symbol maps
4. update the affected `src/` files by hand
5. `tools/progress.py` — re-verify every match still holds

`rename.txt` rows are `<program> <space> <address> <name> # evidence`. The
program and space columns both matter: every overlay loads at `0x800643A0` and
every sub-EXE at `0x80080000`, so the same address means different things in
different binaries. An earlier global symbol map renamed a DNG symbol to
`CatPrim`, a name harvested from `END.EXE`.

Nothing in `src/` uses an address-derived name. Where a name is asserted it is
backed by evidence recorded in `rename.txt` — for example `SndSeqPlay` calls
`SsSetNck`/`SsSeqOpen`/`SsSeqSetVol`/`SsSeqPlay` over data at `0x80180000`,
which is where `main` loads `OPEN.BIN`, making **OPEN.BIN the sequence bank**.

## How the split is driven

spimdisasm's heuristics were not good enough here — `.text` has code and data
interleaved, and it merged distinct functions. The split is therefore driven by
**Ghidra's** analysis:

1. `ghidra/ExportCodeMap.py` dumps per-function extents, defined data and names
   for each program, with overlay blocks as separate address spaces
2. `tools/gen_symbols.py` turns that into splat `symbol_addrs` files. Interleaved
   data cannot be a splat `data` subsegment — splat would move it into `.data`
   after `.text` and change the byte order — so data is emitted as inline
   symbols with explicit `type:`/`size:`
3. `tools/gen_splat_configs.py` writes one splat config per target
4. `tools/gen_undefined.py` resolves references landing inside large data runs

Three things had to be handled to get a clean split, all in `gen_symbols.py`:

- **4-byte alignment.** Ghidra occasionally reports a degenerate 1-byte function
  body, which produced a misaligned data run and made spimdisasm bail on
  everything after it (functions went 1221 → 7).
- **Alignment padding.** Ghidra excludes trailing padding from a function body,
  leaving 4–16 byte islands. Emitting those as data runs demoted every following
  function to a plain label. They are absorbed into the preceding function.
- **Degenerate stubs.** Functions under 8 bytes are dropped so they fall into the
  data runs.

Overlays additionally need `.data` ordered *first* (their id dword is at file
offset 0), `subalign: 4`, and `ld_align_section_vram_end: False` — otherwise
splat pads each section to 16 and the file comes out 12–24 bytes too long.

## Notes

- The PSX SDK should **not** be decompiled. Link the real Psy-Q libraries via
  `psyq-obj-parser` and they match for free. Ghidra's signature database
  identifies which objects are present, which also keeps SDK code out of the
  candidate list — `tools/pick_candidates.py` filters anything Ghidra has named,
  after an early near-miss where `PRNT_OBJ_594` (Psy-Q `printf` internals) came
  top of the list.
- See `docs/p1-memory-map.md` for the overlay mechanism and program structure.
