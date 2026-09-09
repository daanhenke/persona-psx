## Matching Nudges

The overlays were built with **gcc 2.6.0 at `-O2`**. When a candidate has the
right length and the right instructions but puts them in the wrong registers, or
in the wrong order, the fix is almost never to fight the allocator directly - it
is to change the shape of the source so the compiler reaches the original's
answer on its own.

This is a catalogue of the levers that have actually turned a near-miss into a
match in this tree. Every entry cites the file that proves it; those citations
are the evidence, not illustrations. If you add a lever here, cite a function
that matched because of it.

A nudge is only worth keeping if it is invisible in behaviour. Everything below
compiles to the original bytes *and* still says what the routine does. When one
of these is load-bearing, say so in a comment at the site - the next person to
read it will otherwise tidy it away.

---

### Reading the diff first

Before reaching for a lever, know which kind of difference you have:

| What the diff shows | What it usually means |
|---|---|
| Same instructions, different registers | Allocation - block boundaries, one-variable-or-two, statement order |
| An extra test before the loop body | `do/while` where the original has `while` |
| One increment at the bottom, original has several | A missing unreachable trailing statement |
| An extra `lui`/`addiu` pair per access | Walking vs indexing, or a linker symbol where the original used an address |
| A constant re-materialised after a call | The value is being set up on the wrong side of the call |
| Wrong length | Not a nudge. The structure is wrong; go back to the decompiler |

`scripts/local/fdiff.py` normalises everything the linker decides, so a routine
of the wrong length still lines up either side of the divergence. Note that
`objcmp.py` masks relocated words - a MATCH there can still be calling the wrong
extern, and only a full `make build` catches that.

---

### 1. Basic-block boundaries

`do { ... } while (0)` gives a group of statements a block of its own. gcc 2.6
allocates per block, so this is the bluntest and most reliable way to move a
value into the register the original keeps it in.

- [gaugecolour.c:35](/src/btlp/gaugecolour.c#L35) - gives the block the boundary the original keeps its tint in.
- [uploadtim.c:66](/src/btlp/uploadtim.c#L66) - puts the loop's counter *and* pointer where the original has them.
- [objpiecesrot.c:48](/src/btlp/objpiecesrot.c#L48), [objtilesrot.c:47](/src/btlp/objtilesrot.c#L47) - the same trick, for the counter.
- [talkscore.c:67](/src/btlp/talkscore.c#L67) - stops a second copy of the constant being emitted at the end.
- [actors.c:40](/src/adv/game/actors.c#L40) - the returned value needs a block of its own.
- [openmessage.c:66](/src/btlp/openmessage.c#L66), [objects.c:298](/src/btlp/objects.c#L298), [statusframes.c:61](/src/adv/gfx/statusframes.c#L61).

An `if` arm, or a `{ }` block with declarations in it, does the same thing where
one reads better.

### 2. Loop shape

**`while` where you would write `do/while`.** Written as a `do/while`, gcc peels
the first test out above the loop; the original often does not have that peel.

- [demonline.c:101](/src/btlp/demonline.c#L101), [talkanswer.c:102](/src/btlp/talkanswer.c#L102) and [:119](/src/btlp/talkanswer.c#L119), [talkscenegift.c:143](/src/btlp/talkscenegift.c#L143).

**An unreachable statement after the `return`.** Without it gcc shares one
increment at the bottom of the loop; with it, each back-edge gets its own.

- [cursor.c:32](/src/btlp/cursor.c#L32), [cursor2.c:35](/src/btlp/cursor2.c#L35), [targetcursor.c:27](/src/btlp/targetcursor.c#L27).

**`for` instead of `do/while`** avoids the peel in the other direction; the
member search in `begintalk.c` needs it.

**A `goto` where you would write a loop.** gcc lifts values that are invariant
across a loop into saved registers - repeated state numbers, a repeated timer
value - and the original often does not have them there. Written as a label and
a `goto` at the bottom, the block is not a loop the optimiser recognises and
nothing is lifted:

```c
top:
    {
        ...
    }
    if (<carry on>) {
        goto top;
    }
```

- [windowstep.c:79](/src/btlp/windowstep.c#L79) - sixteen bytes over as a
  `do/while` (three constants in `s3`, `s4`, `s5`), four *under* as a goto, and
  a match once the arms were in the right order. A `for (;;)` with a `break`
  behaves exactly like the `do/while`; only the goto breaks the recognition.

### 3. Where a value is set up

gcc 2.6 will not carry a constant across a call. If the original loads it once
and the candidate re-materialises it after the call, move the set-up *above* the
call.

- [menu.c:44](/src/btlp/menu.c#L44) - the count is set up away from its loop, because from below the wait it would have to decrement and test rather than fold.
- [averagesides.c:56](/src/btlp/averagesides.c#L56) - the level total is cleared twice; the second clear is what keeps it in the register the loop reads.

A **division** does the same thing as a call. `%` or `/` on a divisor that is
not a constant expands to `div`, the two trap checks and a trailing `mfhi`, and
a cheap store written *before* it gets scheduled ahead of the `div` - which
reallocates everything downstream. Write the statement carrying the division
first and the constant store after it:

```c
g_btl_cursor_cel = (g_btl_cursor_cel + 1) % anim->cels;
g_btl_cursor_timer = 0;
```

- [cursorplace.c:57](/src/btlp/cursorplace.c#L57) - thirteen words out with the
  two the other way round, and a match this way, whether or not the remainder
  goes through a temporary.

Note that this is not a store-order preference, and reading it as one will send
you the wrong way: the *output* keeps `sb zero` before `sb v1`, the opposite
order to the source that matches.

### 4. One variable, or two

Two locals that never overlap will not necessarily share a register, and one
local reused for two unrelated things often will. Both directions come up.

- [backdrop.c:41](/src/common/gfx/backdrop.c#L41) - one local for the gradient's blue and then the grid's grey; two locals do not compile to this.
- [itemcell.c:51](/src/common/ui/itemcell.c#L51), [itemrowuse.c:12](/src/common/ui/itemrowuse.c#L12) - one scratch carries the id test in and the glyph bank out.
- [formationrepair.c:20](/src/common/game/formationrepair.c#L20) - one counter serves both loops, which only happens if it is the same variable.
- [pickmember.c:74](/src/btlp/pickmember.c#L74) - the mask is a local so it stays in a register across the loop.

The reverse is also a lever: in `BtlBuildOffers` a *shared* temp put the value in
`v0`, clobbering a speculatively-loaded `1` so it had to be re-materialised;
splitting it into its own variable moved it to `v1` and the instruction vanished.

### 5. Walking or indexing

Both forms come up, and the wrong one costs an address computation per access.

- [averagesides.c:61](/src/btlp/averagesides.c#L61) - **indexed**, because with `a++` gcc biases the pointer to the last field it reads and every offset comes out negative.
- [objpiecesrot.c:57](/src/btlp/objpiecesrot.c#L57) - reached through the object every time rather than held in a local, because the count is re-read each turn.
- [formationplace.c:5](/src/common/game/formationplace.c#L5) - an int-taking prototype makes gcc keep the table's base in a saved register instead of folding it into each access.

Where two tables are adjacent, reaching the second as the tail of the first lets
one base address serve both walks - `e = a + BTL_PARTY` in `buildoffers.c`, which
is a whole instruction cheaper than naming `g_btl_enemies`.

### 6. Repetition that must not be tidied

The original often re-reads something a human would hoist. Hoisting it changes
the index arithmetic.

- [items.c:72](/src/common/game/items.c#L72), [itemsedit.c:22](/src/common/game/itemsedit.c#L22) - the base is spelled out again inside the loop.
- [vramqueue.c:55](/src/btlp/vramqueue.c#L55) - the count is read three times over; the second and third reads are what place the index arithmetic.
- [drawnumber.c:13](/src/btlp/drawnumber.c#L13) - the count doubles as the zero the width is tested against; a literal there does not match.

### 7. Statement order

Statements that commute in C do not commute in the output.

- [soundbank.c:82](/src/btlp/soundbank.c#L82) - taking the two addresses in this order; assigning the one used first *first* puts the pair in the wrong registers.
- [objmodel.c:176](/src/btlp/objmodel.c#L176) - two steps on purpose; folding them changes the registers.
- [tilemapruns.c:13](/src/common/gfx/tilemapruns.c#L13) - counter, then destination, then first cell.
- [actors.c:17](/src/adv/game/actors.c#L17) - the y step spelled out in two statements.
- [equipauto.c:47](/src/common/game/equipauto.c#L47) - the count has to be taken between the two.

In `buildoffers.c` the totals had to go `hp`, `hp_now`, `demons`; that ordering
alone was worth eleven words. Incrementing the record pointer before the counter
was worth two more in each of two loops.

### 8. Reading the same bytes differently

- **Casts.** The battle reads the ailment byte as signed where `Char` declares it
  unsigned - [cursor.c:13](/src/btlp/cursor.c#L13), [actor.c:9](/src/btlp/actor.c#L9).
  Splitting a compound test into two also lets gcc fold differently;
  [actor.c:22](/src/btlp/actor.c#L22) keeps it as one.
- **Unsigned switch operand.** A `switch` opens with a range test; on a signed
  operand that is `slti`, on unsigned `sltiu`. If the image has `sltiu`, cast the
  operand - [talkstep.c:201](/src/btlp/talkstep.c#L201).
- **Deliberately disagreeing prototypes.**
  [fadeblock.c:24](/src/common/gfx/fadeblock.c#L24) - the caller was built against
  short parameters, so the prototypes disagree on purpose. The same routine can
  compile differently per overlay for this reason.

`switch` versus an if-chain is identified by that same opening range split: an
if-chain compares each value in turn, a switch tests the range once.

A `switch` that compiles to a jump table puts the table in the unit's `.rodata`,
which means the unit needs its own `.rodata` subsegment in the yaml - and that it
can no longer be guarded with `INCLUDE_ASM`, because the guarded form emits no
table and the link fails.

**The arms come out in source order**, and the jump table records the order the
original had them in. Read the table, sort the cases into the order its entries
point, and write them that way - it is not the numeric order. `BtlWindowStep`'s
run 12, 8/10/11, 9, 2, 3, 4, 6, 5, 7, 1, and it does not match in any other.
Include the arms with no body too (`case X: break;`): a missing low case makes
gcc subtract before the range test, which shows up as `addiu v1,v0,-1` and a
`sltiu` against the wrong bound. Identical trailing code in two arms is merged
by the compiler on its own - you do not write the shared tail yourself.

The padding after a jump table belongs to nobody: gcc emits the entries and the
next subsegment starts later, so the gap needs a bare `rodata` segment of its
own or every following section shifts. See
[windowstep.c](/src/btlp/windowstep.c) and its `- [0x668, rodata, btlp_rodata4a]`.

### 9. Code that does nothing

Sometimes the original contains a block that cannot run, or a test of something
that is never zero. It is still in the bytes, so it has to be in the source.

- [takeparty.c:79](/src/btlp/takeparty.c#L79) - never entered; nothing writes the
  flag and it is zero on disc. gcc emitted the block anyway.
- [moodretire.c:42](/src/btlp/moodretire.c#L42) - an empty `if (g_btl_offer == 0) { }`,
  which is what puts the table's address where the original keeps it.

Confirm these in the bytes before adding them. Ghidra's xrefs miss un-analysed
callers, so use `scripts/local/callers.py` rather than trusting the xref count.

### 10. Addresses

A global whose low half is zero costs one `lui` when it is reached by address and
two instructions through a linker symbol.

- [begintalk.c:95](/src/btlp/begintalk.c#L95) - `#define g_btl_scratch ((u_char *)0x801C0000)`,
  because its low half is zero so one `lui` is the whole of it.
- The same pattern across ADV: [sound.c:22](/src/adv/audio/sound.c#L22),
  [chestresult.c:35](/src/adv/game/chestresult.c#L35),
  [slot.c:12](/src/adv/gfx/slot.c#L12), and others.

Bind a constant to the base term rather than the index: `base + (i * n + k)` and
`(base + k) + i * n` are the same value and different code.

### 11. Stack slots, and which local gets the lower one

When a routine takes the address of several locals and passes them - the classic
case is a GTE call's out-parameters - the *order of the slots* is part of the
match, and it is not decided by declaration order.

The rule gcc 2.6 actually follows is:

> **An aggregate gets a lower slot than any scalar**, whatever order the
> declarations are in.

So `long p; long flag[2];` always comes out with `flag` at the bottom of the
locals and `p` above it, and swapping the two declarations changes nothing. The
original frames in this game are usually laid out the other way round, and that
is what the diff is telling you when two `addiu vN,sp,K` differ. Two corrections
follow from the rule, and between them they close the whole family:

**Merge the out-parameters into one scratch array.** If the image's two
addresses are adjacent - `sp+40` and `sp+44` - they are not two locals at all,
they are two elements of one array:

```c
long scratch[3];
...
RotTransPers4(..., &scratch[0], &scratch[1]);
```

Size the array to the whole locals region, not to what the code reads: the frame
size fixes it, and a spare element is often what the original had.

**Make the address-taken scalars an aggregate too.** A projected point written as
`short sx; short sy;` is a scalar pair and lands *above* the scratch; written as
the `DVECTOR` it really is, it lands below, where the original has it.

Proven on the whole family:

- [objpiecesrot.c:40](/src/btlp/objpiecesrot.c#L40), [objtilesrot.c:41](/src/btlp/objtilesrot.c#L41) -
  **matched** on the merge alone, having been two words out.
- [objtextrot.c:44](/src/btlp/objtextrot.c#L44) - one `long[4]` in place of a
  never-used `RECT` and two longs; seven words to five.
- [objflat.c:71](/src/btlp/objflat.c#L71) - both corrections together, thirteen
  words to four, every slot on the image's.
- [objmodel.c:74](/src/btlp/objmodel.c#L74) - both, plus not holding
  `&o->scale_x` in a local; eight bytes over to the exact size.

Read the slots straight off the diff (`addiu a1,sp,24` against
`addiu a1,sp,44`). They tell you how big each local is, which sits where, and -
with the frame size - how many bytes of locals the original had in total.

### 12. Unguarding can break the link

A body that has been sitting behind `INCLUDE_ASM` calls its callees by whatever
name the asm used. The moment the C is live, the call goes out under the real
name, and if the overlay never had that name the link fails:

    undefined reference to `SetGeomOffset'

The asm was calling it `func_80029F64`. The fix is a line in
`configs/JP1/externs.<target>.ld` giving the real name that address - its
neighbour `SetGeomScreen` was already there. `objcmp` cannot see this coming,
because it masks exactly the words the linker fills in; only `make build` does.

---

### What does not work

Worth knowing so the time is not spent twice.

- **Declaration order.** gcc 2.6 does not allocate in declaration order, and
  that holds for stack slots as well as registers - swapping two locals leaves
  the frame byte-identical. Confirmed four times, most recently across ten
  permutations on `BtlBuildOffers` (no register moved) and on
  `BtlDrawObjPiecesRot` (no slot moved). Change what the locals *are*, not the
  order they are written in; see section 11.
- **The `register` keyword.** Ignored for this purpose.
- **`/=` versus `= x /`,** and the other spellings of the same expression. No
  effect observed.
- **Reordering a loop body** when the loop already has the right shape. If the
  boundary is right, the allocation inside it is decided by the boundary.

### When to stop

If the candidate is the exact right length and the residual is *only* register
naming - especially if it is systematic, like the argument registers being handed
out in the opposite order throughout - hand it to the permuter rather than trying
more spellings:

    sh scripts/local/perm.sh <target> <object-under-src> <Symbol> [jobs]

It found `BtlTalkScoreLine` in 560 iterations after a dozen hand variations had
failed. Progress is written to `permuter/<target>/<Symbol>/` - read it there
rather than piping the run through `tail`, which buffers.
