## RTL Dumps

`odiff` shows which instructions differ. It cannot show which part of the
compiler put them there, and for an ordering or a delay-slot difference that is
the whole question. cc1 can answer it: each pass can write out the RTL it leaves
behind, and two neighbouring dumps show exactly what one pass changed.

Reach for this before sweeping spellings whenever the residual is *where* an
instruction sits rather than *which register* it uses.

---

### Getting the dumps

Build the unit into `build/nm` first, so its preprocessed source exists:

```sh
make NON_MATCHING=1 SKIP_ASM=1 GEN_COMP_TU=1 BUILD_BASE_DIR=build/nm \
     build/nm/JP1/src/btlp/<unit>.c.o
```

Ask make for the exact cc1 line instead of retyping the flags. `-n` prints the
commands without running them and `-B` prints them even though the object is
up to date:

```sh
make -n -B NON_MATCHING=1 SKIP_ASM=1 GEN_COMP_TU=1 BUILD_BASE_DIR=build/nm \
     build/nm/JP1/src/btlp/<unit>.c.o | grep cc1
```

Copy the `.sjis.i` somewhere scratch and run that line on it with `-d` letters
added. Each letter writes `<input>.<pass>` beside the input, so copying first
keeps the dumps out of `build/nm`:

```sh
mkdir -p build/rtl
cp build/nm/JP1/src/btlp/memberact.sjis.i build/rtl/m.i
cd build/rtl
../../tools/bin/gcc-2.6.0/cc1 -O2 -G0 -mips1 -mcpu=3000 -w -funsigned-char \
    -fpeephole -ffunction-cse -fpcc-struct-return -fcommon -msoft-float -mgas \
    -fgnu-linker -fdollars-in-identifiers -quiet -dcS -o m.s m.i
# writes m.i.combine and m.i.sched
```

The letters this cc1 accepts, checked against its output:

| Letter | File | The RTL after |
|---|---|---|
| `r` | `.rtl` | generation from the tree |
| `j` | `.jump` | the first jump optimisation |
| `s` | `.cse` | common subexpression elimination |
| `L` | `.loop` | loop optimisation |
| `t` | `.cse2` | the second cse |
| `f` | `.flow` | flow analysis |
| `c` | `.combine` | instruction combination |
| `S` | `.sched` | instruction scheduling |
| `l` | `.lreg` | local register allocation |
| `g` | `.greg` | global allocation and reload |
| `R` | `.sched2` | scheduling again, after reload |
| `J` | `.jump2` | the last jump optimisation, which does the cross-jumping |
| `d` | `.dbr` | delayed-branch scheduling, which fills the delay slots |

`-drjsLtfcSlgRJd` writes all of them.

### Reading a dump

Insns are separated by blank lines, and each one starts
`(insn UID PREV NEXT pattern ...)`. The UID stays the same from pass to pass,
so one insn can be followed through the dumps and its position compared. Up to
`.lreg` the registers are pseudos (`reg:SI 97`); from `.greg` on they are hard
registers (`reg:SI 2 v0`). In `.dbr` a filled delay slot shows up as a
`(sequence [...])` holding the branch and the insn moved into it.

The dumps are verbose. Folding each insn onto one line makes two of them easy
to compare side by side:

```python
import re
txt = open("m.i.sched").read()
body = re.search(r";; Function BtlMemberMotion06\n(.*?)(\n;; Function|\Z)",
                 txt, re.S).group(1)
for chunk in body.split("\n\n"):
    line = re.sub(r"\s+", " ", chunk).strip()
    if line.startswith(("(insn", "(call_insn", "(jump_insn", "(code_label")):
        print(line[:140])
```

Print a window of a dozen insns before a call you can find by name, once from
`.combine` and once from `.sched`, and the reordering is plain to see.

### Testing a guess without touching the tree

cc1 does not run the preprocessor, so it will compile a hand-written `.c` with
no includes as long as every macro is already expanded. A cut-down copy of the
code in question compiles in well under a second in `build/rtl`, which makes it
quick to ask "does this shape reorder that" - write the variants, compile each,
and compare the assembly cc1 writes.

It is only indicative. The allocation around a cut-down routine is not the real
routine's, so confirm anything it suggests with a sweep on the real unit.

### What the dumps have settled

- **sched reorders a call's argument set-up, except when the call ends its
  basic block.** gcc expands a call as the stack arguments and then `a0`..`a3`;
  sched normally moves the stack argument last. A call that is the last insn of
  its block keeps expand order. See the section on it in
  [Matching Nudges](Matching%20Nudges.md), and `BtlMemberMotion06` in
  [memberact.c](/src/btlp/memberact.c).
- **A label between a store and the call after it** stops sched moving the
  call's argument set-up ahead of the store, and stops dbr pulling it into the
  branch in front. Where the image has the argument in both branches' delay
  slots, the shared tail was written out twice rather than reached by `goto`:
  jump2's cross-jumping folds the copies back together, but only after sched
  has run. `BtlStageCommand`'s handover, 98.24% to 98.89%, in
  [commandstage.c](/src/btlp/commandstage.c).
- **The load in front of `ovl_btlp_entry`'s first texture upload** is placed by
  sched, and every placement of the `g_btl_tim_buf` store in the source gives
  the same scheduled order. Still open; 16,000 permuter iterations found
  nothing under the base score either.
- **`BtlStageCommand`'s `run:` block** is placed by sched2. combine and sched
  leave `g_btl_step = 0` ahead of the table index's `sll`; after reload sched2
  moves the `sll` first, and dbr then copies it into step 5's delay slot. The
  image's sched2 leaves them alone. Still open - see the trace below for why
  ours moves it.

### How the scheduler decides

gcc 2.6.0's own `sched.c` is in the release tarball that
[decompals/old-gcc](https://github.com/decompals/old-gcc)'s Dockerfiles fetch;
unpack it under `build/` to read it. The rules, as it implements them:

- It works one basic block at a time, **backwards from the end**, placing each
  chosen insn in front of the ones already placed.
- An insn's **priority** is the longest dependency path leading to it inside
  the block, each step weighted by the latency of the insn depended on. On the
  r3000 costs this cc1 uses, a load takes 2 cycles and a store or anything else
  1, so only loads ever lengthen a path.
- Ready insns are sorted by priority, then by their relation to the insn just
  placed (depends on it with a latency above one, depends on it otherwise,
  independent - the independent ones go nearest), then by **original insn
  order**, so a pure tie leaves the source's order alone.
- Within the top priority group, `schedule_select` takes the insn with **the
  greater potential hazard** - one using a function unit, so a load or a store,
  ahead of a register operation.
- The first pass, before reload, also lifts an insn that first sets a pseudo
  to the top priority. sched2, after reload, does not.

The `.sched` and `.sched2` dumps print every decision: `;; ready list at T-n`,
`launching N before M`, `blocking insn N`, `insn N has a greater potential
hazard`. For `BtlStageCommand`'s `run:` block the first pass keeps the image's
order because the `sll` sets a new pseudo; sched2 sees the `sll` and the store
tied at priority 1, and the store's potential hazard puts it next to the load -
our order.

Two settings are ruled out. `-mcpu` only changes a load's latency (the default
here is the r3000's 2); the other CPUs reorder these blocks differently, not
into the image's order. And the original did run both passes: with
`-fno-schedule-insns2`, units that already match change by hundreds of lines
even with label names normalised.

### What cut-down compiles have settled

These came from small hand-written functions compiled on their own, then
confirmed on the real unit.

- **An array member reached through a pointer adds the base first.**
  `base->rows[k].entry[j]` builds `base + k*size`; `(base->rows + k)->entry[j]`
  builds `k*size + base`. The `.rtl` dump shows it at generation - no later
  pass swaps two registers. `BtlTalkSceneDemand` uses both, one per site, and
  the image says which.
- **A field's address used once in an update is built at the top of the
  block.** `g_btl_offer[slot].mood[gauge] += weight` materialises
  `g_btl_offer+0x3E` before the slot is even loaded; through
  `short *mood = g_btl_offer[slot].mood;` it is built just before it is added
  in, as the image has it. `BtlTalkSceneDemand`, 99.13% to 99.57%.
- **The scratch script's address** - see the section on it in
  [Matching Nudges](Matching%20Nudges.md).
