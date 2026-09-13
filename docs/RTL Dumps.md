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
- **The load in front of `ovl_btlp_entry`'s first texture upload** is placed by
  sched, and every placement of the `g_btl_tim_buf` store in the source gives
  the same scheduled order. Still open.
