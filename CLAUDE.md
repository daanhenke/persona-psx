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
  Ghidra has named for exactly this reason.

## Naming

Ghidra is the source of truth. Add rows to `config/p1-jp/rename.txt`:

    <program> <space> <address> <name>   # evidence for the name

Both the program and space columns matter — every overlay loads at `0x800643A0`
and every sub-EXE at `0x80080000`, so one address means different things in
different binaries. Then:

1. run `ghidra/ApplyNames.py` in Ghidra (via ReVa `run-script`)
2. run `ghidra/ExportCodeMap.py` in Ghidra
3. `tools/gen_names.py`
4. update the affected `src/` files **by hand**
5. `tools/progress.py` to confirm nothing regressed

Renaming an already-named symbol will not propagate itself; step 4 is manual and
skipping it silently desynchronises the source from the target.

The `# evidence` column is not decoration. Every asserted name should be
traceable to something observed — an SDK call it wraps, a constant it writes, a
sentinel value, a caller's behaviour.

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
- Not every translation unit uses the same flags. `ATLUS.EXE` and `OPEN.EXE` are
  mostly SDK plus `-O0` code with a small-data area; use
  `/* cc1flags: -O0 -G8 */` and record the binary's `$gp` in
  `config/p1-jp/gp.txt` (it is set at the entry point, not in the PS-EXE header).

## Layout

    bin/            downloaded toolchain (gitignored; see bin/*.sha256)
    config/p1-jp/   splat configs, symbol maps, rename.txt, gp.txt, ghidra/*.json
    docs/           memory map and program structure
    ghidra/         scripts run inside Ghidra via ReVa
    include/psyq/   Psy-Q SDK headers
    src/p1-jp/      decompiled C, one directory per target
    tools/          the pipeline, plus vendored maspsx / m2c / asm-differ / permuter
