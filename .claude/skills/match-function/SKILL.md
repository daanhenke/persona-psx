---
name: match-function
description: Take a Persona 1 function from unmatched asm to byte-matching, named C. Use when asked to decompile, match, or "do a function" in this repo, when a candidate is stuck below 100% objdiff, or when picking what to work on next.
---

# Matching a function

The loop is: pick a candidate → understand it → write C → match → name it →
record it. A function is not done until it is 100% *and* every symbol it touches
has a real name.

Run everything through WSL with `PYTHONDONTWRITEBYTECODE=1`, and clear
`tools/__pycache__` after editing `tools/` (see CLAUDE.md).

## 1. Pick a candidate

    tools/pick_candidates.py 3 24 48

Skips SDK functions (anything Ghidra has named), soft-float, GTE, jump tables,
and translation units built with different flags (frame pointer or `$gp` usage).
Prefer something with real control flow and a couple of calls. Avoid leaf
one-liners — they teach nothing and inflate the count.

Before committing to one, check whether it is shared across targets:
mask `jal`/`j` targets and immediates, then compare bytes. `atlus`/`open`/
`movie`/`end` and the `dng`/`s2d`/`adv` trio share a lot, so one source can
cover several targets.

## 2. Understand it

Get Ghidra's decompilation via ReVa (`get-decompilation`). For overlay functions
the symbol form is `FUN_OVL_DNG__80078ee8`; sub-EXEs are separate programs
(`/Persona 1/EXE_END.EXE`).

Then read the actual asm — `tools/mfunc.py <target> <symbol>` shows size, and the
split asm is in `asm/p1-jp/<target>/<target>.s`. Ghidra's `goto` rendering often
hides a shared tail or an `||`, and the asm tells you the real shape.

Chase the callees far enough to name them. That is not optional extra work; it
is the deliverable.

## 3. Write the C

Put it in `src/p1-jp/<target>/<name>.c`, include from `include/psyq/`, and use
real names throughout. If a translation unit was built differently, add
`/* cc1flags: -O0 -G8 */` at the top.

    tools/mfunc.py <target> <symbol> src/p1-jp/<target>/<file>.c --diff

objdiff's percentage is the verdict. `--all` shows matching rows too.

## 4. Close the gap

Read the diff before changing anything. Patterns seen repeatedly here:

| symptom | cause |
|---|---|
| extra `andi vN, vN, 0xff` in target | callee returns `u_char`, not `int` |
| a `nop` in target where you emit a store | target loads before storing; MIPS1 load-delay slot |
| value reloaded each iteration in target | it lives in a saved register — hoist it to a local before the loop |
| stores duplicated per branch | target shares a tail; express it with `goto` |
| backward conditional vs unconditional `j` | loop layout: try `goto` chains, `for(;;)`, `do/while` |
| address computed then loaded, vs `lw v1, 8(v0)` | struct member access, not array indexing (visible at `-O0`) |
| target frame is larger, nothing else differs | gcc 2.6 allocates declared locals even when unused — add one of the right size (`CdlFILE` = 0x18, `CdlLOC` = 4) |
| target re-loads a global you just stored | the global is `volatile`; gcc otherwise reuses the register |
| target keeps a redundant test you write but gcc collapses | usually means the value is re-read; check whether `volatile` belongs on it |
| `addu vN, sM, zero` before a store | the pointer is also the **return value** — the function returns it |
| target says `D_800622FC`, you say `g_cd_queue+0xc` | reference into a named object; `normalize_asm` rebases these automatically |

`volatile` is a real modelling decision, not a matching trick: apply it where the
variable genuinely changes under a callback (`g_cd_busy`, `g_cd_queue_count`),
and *not* where it does not (`g_cd_queue_index` is only written from the submit
path — marking it volatile fixed one function and broke three).

If the structure is identical and only register allocation differs, stop hand
guessing and use the permuter:

    tools/setup_permuter.py <target> <symbol> <cand.c>
    tools/decomp-permuter/permuter.py permuter/<target>/<symbol> -j12 --stop-on-zero --best-only

Run it in the background and work on something else. When it lands, fold the
insight back into readable C rather than pasting its output — its edits are often
noise, and the useful ones are usually a deliberate temporary or a redundant
assignment that forces a register. Keep a comment saying why, so nobody
"simplifies" it away.

A permuter score of 0 and a 100% objdiff should agree. If they disagree, the
harness normalisation is wrong, not the C.

## 5. Name everything

No `func_XXXXXXXX` or `D_XXXXXXXX` may remain in `src/`. Add rows to
`config/p1-jp/rename.txt` with evidence, then apply them (see CLAUDE.md for the
five steps). Name from what you can observe:

- what SDK calls it wraps
- constants it writes (`0x100` unit scale, `0xFF` "none" sentinel, frame counts)
- how callers use the result
- which shared globals it touches, and who else touches them

If you genuinely cannot tell, investigate more. Do not ship a confident-sounding
guess, and do not leave the address form.

## 6. Record it

Add a row to `config/p1-jp/decomp.txt`, then:

    tools/progress.py --record

That re-verifies every previously matched function too, which catches renames
that were applied in Ghidra but not carried into a source file.

Finally confirm `make check` is still byte-exact for all 11 targets.
