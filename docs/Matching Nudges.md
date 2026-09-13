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
| Right instructions in the wrong order, or a different delay-slot filler | A pass made that choice; ask cc1 which one with [RTL Dumps](RTL%20Dumps.md) before sweeping spellings |

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

**The whole routine inside one `for (;;)`.** A stage that waits for something
before it will do anything reads naturally as a wait loop followed by the work:

```c
while (<not ready>) {
    BtlDrawFrame();
}
<the work>
```

but gcc rotates that - the frame lands directly under the prologue with a `j`
into the test. The image instead has the test at the top and the frame at the
very end, past everything, which is the shape of the whole routine being the
loop and the work being one `if` inside it:

```c
for (;;) {
    if (<ready>) {
        <the work; every way out is a return>
        return;
    }
    BtlDrawFrame();
}
```

That also makes anything the work uses invariant across a loop, so gcc lifts it
into a saved register in the prologue - which is the tell. `BtlStageOpen` has
`li s3,0x11` before the frame is even set up, for a `BTL_STATUS_DOWN` compare
several blocks down.

- [roundflow.c:88](/src/btlp/roundflow.c#L88) - matched on the second try once
  the wait became the loop; the `li s3,0x11` in the prologue was what said so.
  [roundflow.c:189](/src/btlp/roundflow.c#L189) is the same shape with a
  `switch` inside it.

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
**Do not stack the empty arms onto one label.** Written as

```c
case 0x41: case 0x42: case 0x43: ... case 0x46:
    break;
```

all six share a label, and gcc folds adjacent nodes with the same label into a
single case node before it decides how to dispatch. It uses a jump table only
when the range is at most ten times the node count, so folding twenty-one nodes
down to twelve turned a range of 160 from dense enough into too sparse, and
8584 bytes of `BtlChooseEnemyMove` came out as a binary compare tree instead.
One `case X: break;` per line keeps the nodes apart and the table comes back.

- [roundflow.c](/src/btlp/roundflow.c) - 673 instructions as a tree against the
  image's 618; 634 the moment the arms were split.

Include the arms with no body too (`case X: break;`): a missing low case makes
gcc subtract before the range test, which shows up as `addiu v1,v0,-1` and a
`sltiu` against the wrong bound. Identical trailing code in two arms is merged
by the compiler on its own - you do not write the shared tail yourself.

The padding after a jump table belongs to nobody: gcc emits the entries and the
next subsegment starts later, so the gap needs a bare `rodata` segment of its
own or every following section shifts. See
[windowstep.c](/src/btlp/windowstep.c) and its `- [0x668, rodata, btlp_rodata4a]`.

**A byte kept as a byte.** A `u_char` local used in an int context is
zero-extended at each use - `andi v1,a0,0xff` next to arithmetic that uses the
raw register - while an `int` or `u_int` local is extended once and the masks
vanish. If the image re-masks the same value more than once, it was held as a
byte:

```c
u_char spell = a->spell[i];           /* andi at each test  */
if ((u_int)(spell - 0x75) < 0x17)     /* the cast keeps the compare unsigned */
```

- [roundflow.c](/src/btlp/roundflow.c) - `BtlChooseEnemyMove`, sixteen bytes
  over as a `u_int`, exact as a `u_char`.

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

**A frame that is bigger than the locals explains it.** The frame is the
outgoing arguments, plus the locals, plus the saved registers, rounded to
eight. If the built routine is short by a multiple of eight and no stack slot
is touched, the original had locals gcc here has none of - and an unused
aggregate reserves the slot where an unused scalar is dropped:

```c
SVECTOR unused;   /* eight bytes the image reserves and never writes */
```

Read the size off the `.frame` directive in the built `.s`
(`# vars= 8, regs= 5/0, args= 24`) rather than counting the prologue.

- [roundflow.c:190](/src/btlp/roundflow.c#L190) - `vars= 0` against the image's
  eight; `BtlStageClose` was 0x68C against 0x694 until the eight bytes were
  there.
- [cursorplace.c](/src/btlp/cursorplace.c) - the same thing at 56 bytes.

### 12. Unguarding can break the link

A body that has been sitting behind `INCLUDE_ASM` calls its callees by whatever
name the asm used. The moment the C is live, the call goes out under the real
name, and if the overlay never had that name the link fails:

    undefined reference to `SetGeomOffset'

The asm was calling it `func_80029F64`. The fix is a line in
`configs/JP1/externs.<target>.ld` giving the real name that address - its
neighbour `SetGeomScreen` was already there. `objcmp` cannot see this coming,
because it masks exactly the words the linker fills in; only `make build` does.

### 13. Check the types before keeping a workaround

The four units below were checked with gcc 2.6.0 and then against the linked
BTLP image, including their relocated calls and tables.

- [menuupdate.c](/src/btlp/menuupdate.c) - `BtlInputKeys() & mask` keeps the
  result in `v0`; `mask & BtlInputKeys()` uses `v1`. Both up/down checks need
  the first form.
- [bgm.c](/src/btlp/bgm.c), [soundbank.c](/src/btlp/soundbank.c) - declaring
  `BtlSePlay` with an `int` sequence matches both the caller and the callee.
  The libsnd calls narrow it inside the callee. A `short` caller declaration
  replaces the full-word bit-31 mask with a signed halfword load. No alternate
  header declaration is needed. Writing the absolute and relative sequence
  assignments directly in their respective arms also fixes the addition's
  register order.
- [talkpair.c](/src/btlp/talkpair.c) - the first-bit index becomes the start of
  the partner search. Reusing it as the *distance* instead was the wrong
  lifetime. The local `{ 0, 4, 7, 9 }` initializer owns 16 bytes of `.rodata`.
- [talkanswer.c](/src/btlp/talkanswer.c) - the answer table is writable `.data`.
  Removing `const` fixes the message-group load's scheduling and makes the
  old `do/while (0)` workaround unnecessary. Initializing the full-moon search
  counter inside that branch fixes its exit destination: outside the branch,
  gcc targets an extra moon-state reload. `fdiff` blanks branch destinations,
  so inspect the actual branch word as well. This unit now owns all 132 bytes
  of its adjacent partner-order, answer, and pair-mask tables.

---

### What does not work

Worth knowing so the time is not spent twice.

- **Declaration order.** gcc 2.6 does not allocate in declaration order, and
  that holds for stack slots as well as registers - swapping two locals leaves
  the frame byte-identical. Confirmed four times, most recently across ten
  permutations on `BtlBuildOffers` (no register moved) and on
  `BtlDrawObjPiecesRot` (no slot moved). Change what the locals *are*, not the
  order they are written in; see section 11.
- **Flipping plain commutative operands.** `a & b` against `b & a`, and the same for
  `x <= y` against `y >= x`, is usually canonicalised before register allocation.
  Eight flips across `BtlChooseEnemyMove` - masks, both random rolls, the odds
  compare, a level difference - left all 618 instructions and all 319 differing
  words exactly as they were. A function call can change this: see the
  `BtlInputKeys()` example in section 13.
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

## `ori` in the low half means a literal, `addiu` means a symbol

A full address materialised as `lui reg,0x801F / ori reg,reg,0x5354` is a number
in the source; the same address through a symbol is `lui reg,%hi(sym) /
addiu reg,reg,%lo(sym)`. splat prints the symbol either way for a `lui` paired
with a *load*, so the pairing that tells the two apart is the one that takes the
address into a register of its own.

`BtlDebugMenu` has both spellings for the same byte. The line it puts up before
the loop reaches it as `g_map_unk4`, with the relocation that comes with that;
the step that reads and writes it every time round reaches it as
`*(u_char *)0x801F5354`, which is the only way the whole address lands in a
saved register. Written through the extern throughout, the address is rebuilt at
each use and the routine comes out a callee-saved register short.

- [debugmenu.c](/src/btlp/debugmenu.c) - exact with the two spellings, 94.19%
  with either one alone.

## An `int` local where the record has a byte

gcc knows a `u_char` loaded with `lbu` is not negative and narrows a signed
comparison against a constant into `sltiu`. Where the image has `slti`, what
puts it back is taking the byte into an `int` of its own first:

    move = o->actor->move;
    if (move >= 0xA3 && move != 0xDB) {

- [personamotion.c](/src/btlp/personamotion.c) - `BtlPersonaMotion02`, exact on
  that alone. This is the opposite direction to the `sltiu` case in section 8:
  read which one the image has and pick the spelling for it.

## Two arms that differ only in a constant share their tail

Compute the *part that differs* in the arms and combine it once afterwards:

    if (half) { step = drop / 26; } else { step = drop / 53; }
    o->scale_y = drop - step;

Spelled `o->scale_y = drop - drop / 26;` in each arm, the subtraction is
duplicated into the branch's delay slot instead of being shared at a label the
two arms meet at.

- [personamotion.c](/src/btlp/personamotion.c) - `BtlPersonaMotion03`, 92.71% to
  99.35% with that and a local of its own for the second scale (section 4's
  split direction: the value and the result are two variables, not one). The
  residual is three words, and it is a choice between two of them - loaded
  before the test the register is the image's but gcc lifts the divide's sign
  fixup out of the arms, loaded inside the arm the fixup stays where the image
  has it but the load wants a move. Seven shapes and 29,000 permuter iterations
  found nothing under the hand version.

## A constant kept in a local of its own

`BtlShowReadyMarkers` walks the party's records by slot and its marker byte by
a pointer, and the last three words of it were the order of the loop's setup:
the image sets the marker code up before the pointer, and we set it up after.

Everything a loop hoists lands in the preheader, which is emitted *after* the
statements the source puts in front of the loop - so a hoisted constant can
never come out ahead of a pointer the source assigns. Writing

    up = MARKER_UP;
    mark = &g_btl_actors[0].marker;

instead of using `MARKER_UP` in the body keeps the constant out of the
preheader entirely: it becomes an ordinary assignment, emitted where the
source puts it, and the order is the image's. That was the whole difference
between 3 words and a match.

Read the other way, a preheader that sets things up in an order the source
cannot produce is the tell that one of them is a plain local, not a constant
the compiler lifted.

## Only one field walked by a pointer

The same routine was 42 words out before that, because it was written as one
byte offset into the record and every field read through it. The image indexes
the record by slot for everything - `lui at; addu at,at,off; lbu v0,%lo(f)(at)`
- and keeps exactly one walking pointer, for the byte it touches three times.

Writing it that way round, `g_btl_actors[slot].field` for the reads and a
`u_char *mark` for the one field that is written, gives the image's four saved
registers. Written as a byte offset throughout, gcc strength-reduces a second
field into a pointer of its own and the whole allocation moves up a register.

## Which way a table index is added up

`BtlPlaceFormation` reaches a two byte entry in a table of encounter blocks.
Written as one index,

    place = &g_btl_place_lo[key * 2 + g_btl_encounter * 0x14];

gcc adds the two products together and then the base, and the routine comes
out one instruction long. Written as pointer arithmetic in the order the
original adds them,

    place = g_btl_place_lo + g_btl_encounter * 0x14 + key * 2;

the base and the block go together first and the character's pair is added
last, which is a match. Left to right association is not something gcc
re-associates away here, so the spelling is load-bearing: read the adds off
the image and write them in that order.

## A two-armed dispatch is a switch, not an if chain

`BtlTalkersJoin` picks between the melee weapon and the gun on one byte that
is 0 or 1. Written as `if (x == 0) ... else if (x == 1) ...` gcc inverts the
first test and drops the third branch: `bnez v1,A / li v0,1`, two instructions
short. Written as `switch (x) { case 0: ... case 1: ... }` it emits the tree
the image has - `beqz v1,MELEE / li v0,1 / beq v1,v0,GUN / j NEXT` - and it
matches.

Read it off the image: a chain tests the first value and falls through, a
switch tests every listed value and then jumps to the default. Three branches
for two arms means a switch.

The same holds for three answers out of range of a jump table: a binary
compare tree (`beq -2`, `slti -1`, `beq -0x100`, then `bne -1` on the other
side) is a switch, and the arms it jumps to are laid out in the order the
cases are written - which the image tells you by where each arm's return
value is loaded.

- [commandpersona.c](/src/btlp/commandpersona.c) - `BtlCommandChangePersona`,
  94.13% as an if chain, 97.51% as a switch with the abort case first, and
  exact with the cancel case ahead of it.

## Two loop steps by the same amount lift the constant out

A constant that needs a `lui` is lifted into a saved register when the loop
uses it **more than once**, and left where it is when it is used once. With one
use `m->savings` is 1 and gcc's loop pass declines; with two it takes it, and
the routine comes out a whole callee-saved register - and often a stack slot -
over the image.

Two rows of an effect grid stepping by the same amount is exactly that shape:

    y_party -= FX_GRID_DY;    /* one constant, two uses: lifted   */
    y_enemy -= FX_GRID_DY;

Cut either line and the `lui` drops back inside the loop and the frame matches.
Spelling the second one differently does not help - `PLACE_ROW_H * PLACE_FIXED`
folds to the same value and cse shares it before the loop pass runs.

- [fxsheet3.c](/src/btlp/fxsheet3.c) - 87.47%; one register and two
  instructions over, and every register above it shifted by one.
- [fxsweep.c](/src/btlp/fxsweep.c) - the same thing with the row's own delay,
  `(FX_SWEEP_H - 1 - row) * FX_SWEEP_STEP`, which is invariant in the column
  loop. Taking it into a local of its own first stops the *product* being
  lifted - 3.80% to 78.96% - but the subtraction still goes.

What does work is anything that puts the register back under pressure, which is
why the same source shape matches inside a bigger routine:
[fxgrid.c](/src/btlp/fxgrid.c) has the same two steps and does **not** lift the
constant, because the move-9 test already holds a saved register.

A giv is not the same thing: a variable the loop both sets and increments -
`x = FX_GRID_X0` at the top of the outer loop and `x += FX_GRID_DX` in the
inner - is set twice and can never be lifted. Writing it out as
`col * FX_GRID_DX + FX_GRID_X0` instead makes it a compiler-built giv whose
*base* is a single-set constant, and that gets lifted instead. Read the inner
loop's preheader to tell which the image has: the source's own assignments come
out in source order, and the compiler's givs after them.

## Hoisting a constant is decided by where its uses are, not how many

`BtlTalkersLeaveField` is one instruction from a match and the difference is
that gcc lifts the `1` that puts the marker up into a saved register, where the
image writes it out afresh at each use. It is not a use-count threshold: the
image has *four* uses of 1 and does not lift it, while ours has three and does.
Two of the image's four sit on the refusal path, so lifting would stretch a
register across the whole loop to save nothing on the common one.

Nothing at the source level moved it: statement order, taking the kept values
into locals first, testing the marker through the pointer instead of by slot,
putting the spell case first. Worth knowing so the next one is not re-fought
from the start.

## Which arm falls through

gcc lays the `then` branch out as the fall-through and jumps away to the
`else`, so the polarity of a branch in the image says which way round the
source had it. `BtlFormatHexGlyphs` took four goes on that alone: the image
tests `n != 0` and jumps to the arm that sets the run flag, so the source is

    if (n == 0) {
        if (seen != 1) { ...blank...; continue; }
    } else {
        seen = 1;
    }
    *out = digit[n];

and the last of its three tests wanted swapping too - `if (i == 0)` with the
blank as the else, not the other way round. Nothing about the code changes;
only which case is written first. Read the branch polarity off the image
before writing the if, and it is one attempt instead of four.

- [objmotion4.c](/src/btlp/objmotion4.c) - `BtlObjMotion02`, 87.21% to exact.
  Both of its nested tests are `bnez piece` to the piece's script with the
  front record's as the fall-through, so both are written
  `if ((attr & MARKER_PIECE) == 0)` first.

## A pointer taken again each turn of the loop

Indexing an array by a monotonic counter gets strength-reduced: gcc keeps a
walking pointer and drops the index. Where the image computes `base + i` fresh
every iteration instead, the source is holding the base in a pointer that is
*assigned inside the loop*:

    p = text;
    do {
        if (p[i] != BLANK) { p[i] += ZERO; }
        i++;
        p = text;              /* this is what stops the walk */
    } while (p[i] != 0xFF);

Assigned once before the loop it is invariant and gets hoisted, and the walk
comes back. That one line took BtlEffectDrawNumber from twelve bytes short to
the right length.

## The permuter does not weigh the frame

Its scorer penalises a stack-layout difference by 1 and a register difference
by 5, so a candidate that has the frame eight bytes wrong can score better than
one that has the frame right and two registers swapped. On BtlEffectDrawLines
its best score of 10 was further from a byte match than the hand version at 8
differing words.

Take its findings as hints about *shape* - it is very good at spotting that a
value wants a second variable, or that a local is doing two jobs - and measure
them with objcmp rather than trusting the score.

## Let the compiler share the successful loop exits

In `BtlChooseEnemyMove`, six searches finish by enabling a move. A `goto` from
each search to one shared store produced the right instructions, but swapped
the actor pointer and the constant 1 between `s3` and `s4`. Writing the store
and `break` inside each search lets gcc merge the tails itself and fixes all
30 affected words. Identical final control flow can still come from source
loops with different register lifetimes.

The remaining two words were `addu v0,v1,v0` where the image has
`addu v0,v0,v1`. Both table lookups need a separate integer row address:

```c
u_long row = (u_long)ai[a->c.key];
p = (u_char *)(g_btl_ai_set * sizeof(BtlEnemyAi) + row);
```

Taking the row first preserves the load order; adding the offset to that
integer address preserves the operand order. Ordinary pointer indexing and
putting the whole integer expression in one statement each lose one of those
properties. See [roundflow.c](/src/btlp/roundflow.c).

## Step a hand-walked pointer after the counter, not before

`BtlOpenMemberBoards` walks a block of sprites by hand and counts the boards
separately. Written with the step before the counter it came out at 94%; with

    i++;
    bars += BOARD_SPRITES;

it is exact. Nothing about the two statements depends on the other - what
changes is which one the delay-slot filler has to hand at each call. The image
takes the counter for the first call's slot and the pointer for the last, and
it can only do that if the counter is emitted first.

Worth trying whenever a loop has two or more things stepping and the diff is a
pile of `addiu` in the wrong delay slots.

## Reading a packed halfword: load, use, load again

An inventory entry packs an id into the low nine bits and a count into the
seven above. The image reads it twice - once for the id and the store, once
for the count - and the order the C has to be written in to get that is

    entry = *slot;                   /* load */
    id    = entry & ITEM_ID;         /* use  */
    count = *slot >> ITEM_SHIFT;     /* load again */

Put the two loads next to each other and gcc folds them into one; compute the
count from `entry` and it folds too. The `use` between them is what keeps them
apart. This is the whole of the three item searches and both list builders.

The second half of the same lever: hold the part you compare in an `int`, not
the `u_short` it came out of. The image's `id < 0x56` is `slti`, a signed
compare, and a `u_short` local gives `sltiu` instead.

## A switch is laid out by case value

gcc emits the case bodies in source order and the dispatch in its own order, so
a three-case switch whose tree tests 1, then 0, then 2 still wants its bodies
written 0, 1, 2. `BtlDrawIndicator` was 83% with the bodies in dispatch order
and 94% with them in value order, before anything else was touched.

## The permuter's score is coarse

A base score of 130 was two instructions out of place in `BtlDrawIndicator`,
and it found the answer - one field assigned before another - in a few minutes
after half a dozen hand variations had failed. A run that sits at 150 is not
necessarily far away, so do not read distance into the number; and a candidate
it finds is usually one line surrounded by noise, so read the diff rather than
taking the source it writes.

## Which local is written first decides both their registers

A loop that reads a record by slot and one of its fields twice gets *two*
walkers out of gcc - one address giv biased to the field, one plain byte
offset - and neither is a variable in the source. Which register each gets
follows the order the loop's own set-up is written in, and nothing else moves
them.

    i = 0;
    found = 0;          /* this way round: exact                        */

    found = 0;
    i = 0;              /* the same code, the two walkers swapped, and
                           nine words out                               */

- [restorefield.c:56](/src/btlp/restorefield.c#L56) - 95.04% to 100% on that
  line alone.
- [targetpick.c](/src/btlp/targetpick.c) - the same in all three pickers,
  where the counter also has to come before the constants the loop hoists.

Writing the walker out by hand as a `BtlActor *` alongside the index gets the
registers right and then emits its own increment on the wrong side of the
compiler's, so it is one word out the other way. Let gcc make both.

## Write both arms out, or write one and branch - not the middle

When two arms of an `if` end in the same code, there are two shapes and the
image tells you which:

**Both arms whole.** gcc merges the tails itself, scanning back from the end
and stopping at the first instruction that differs - which is exactly where
the image's duplication ends.

    if (pickable) { ...; obj->rgb_to[0] = LIT; ...; BtlObjSetFade(obj, 8); }
    else          { ...; obj->rgb_to[0] = DIM; ...; BtlObjSetFade(obj, 8); }

- [pickenemy.c:89](/src/btlp/pickenemy.c#L89) - with the colour in a local and
  the stores shared instead, the call's argument set-up is merged into the
  tail as well and the routine is two words short.

**One copy and a `goto`.** Where the shared block is entered from somewhere
that is not an arm of the same `if` - another `switch` case, or a second exit -
gcc will not merge two spellings of it at all.

- [commandstage.c](/src/btlp/commandstage.c) - four shared exits written out
  in each arm came to 81% with fifty-two instructions too many; one copy and a
  `goto` to it is 96%. The shared block lives in the arm the image puts it in,
  which is not always the first one.
- [targetpick.c](/src/btlp/targetpick.c) - BtlPickTargetMember's cancel and
  abort share a tail the same way.

## A shared answer is the last statement, not a return inside the loop

A wait loop whose success path answers 1 puts that constant in the block
before the epilogue, and the failing answers keep their own:

    for (;;) {
        if (confirm) { ...; break; }
        if (cancel)  { ...; return -1; }
        if (abort)   { ...; return -2; }
        BtlDrawFrame();
    }
    return 1;

Written as `return 1;` inside the loop it is three words out, and gcc folds
`if (x) return 1; return 0;` into an `sltu` before the allocator ever sees it.

- [targetpick.c](/src/btlp/targetpick.c), [opendialogue.c](/src/btlp/opendialogue.c) -
  BtlOpeningLineWanted is exact only with the early returns inside the switch
  and the shared answer after it.

## A frame drawn before the count, or after it

`do { i++; BtlDrawFrame(); } while (i < n);` and
`do { BtlDrawFrame(); i++; } while (i < n);` are the same wait and different
code: which one it is decides what the compiler has free at the loop's guard,
and therefore what lands in its delay slot.

- [openscenes.c:110](/src/btlp/openscenes.c#L110) - one instruction over as
  the first, exact as the second; the permuter found it in minutes after the
  loop shape, the operand order and a second counter had all been tried.
- [opendialogue.c](/src/btlp/opendialogue.c) - the same line, found
  independently, in the one wait of six that has it.

Worth trying on any frame wait that is one instruction out.

## The base binds to the last term written

Pointer arithmetic over a table is not associated the way it is written: gcc
adds the base to whichever term comes *last*, so the image's add order tells
you the order to write them in - backwards.

    rec = table + enc * ROW + a->c.key;   /* base + key first, then the block */
    rec = table + a->c.key + enc * ROW;   /* base + block first, then the key */

- [scriptedaction.c:98](/src/btlp/scriptedaction.c#L98) - both routines were
  82% until the terms were turned round.

Where the image forms the same partial sum in two steps, split the expression
across a second local rather than assigning the same pointer twice - assigning
it twice puts the partial sum in the pointer's own register instead of a
scratch one.

The same rule decides the `addu`'s **operand order** when a constant folds into
the load's own offset and only two terms are left. Read it straight off the
image:

    which = species[ATTACK + a->unkBA];     /* addu v0, index, base */
    which = (species + a->unkBA)[ATTACK];   /* addu v0, base, index */

- [enemymotion6.c](/src/btlp/enemymotion6.c) - one instruction out of sixty-one,
  and exact once the pointer was the first term written.
- [debugequip.c](/src/btlp/debugequip.c) - 96.19% to 98.94% on
  `equip = c->equip; cell = &equip[slot];` rather than `&c->equip[slot]`: the
  image adds 0x20 to the record and then the slot, not the slot and then the
  record.

## Two symbols, not one plus an offset

Tables that sit inside one unnamed run get compiled as offsets from each
other: a counter's address materialised once and the table derived as
`counter - 2736`. Naming each of them in the sym file is what puts them back
to a `lui`/`addiu` pair apiece.

- `g_btl_turn_script`, `g_btl_opening_lines`, `g_btl_member_actions`,
  `g_btl_enemy_lines`, `g_btl_enemy_actions` and `g_btl_enemy_line_next` were
  one 0x10B8-byte `D_800CE5BC`.
- A loop over `table[i + 5]` folds the five into its strength-reduced counter
  (`ori s1, zero, 0x68` and `%lo(table)`). An image that keeps the offset in
  the symbol (`%lo(table+0x28)`) with the counter starting at `i * 8` is
  reading a table of its own that begins there. `g_btl_fx_spread` was
  `g_btl_fx_spread_90` and `g_btl_fx_spread_91`: [fxspell91.c](/src/btlp/fxspell91.c),
  `BtlFxStart91`, 99.74% to exact.

The same applies to sizes: `g_btl_enemies` was sized to one record where the
table is nine, and every reference past the first came out as a fresh
`D_8005Dxxx` rather than `g_btl_enemies+N` - forty-four of them.

## Routines go in the file in the order the image has them

A unit's objects are laid out in source order, so two routines written the
wrong way round are linked at each other's addresses. objdiff pairs symbols by
**name**, so it reports both as matching and says nothing; only the link
catches it, and the tell in the binary diff is a whole unit differing plus a
single byte inside an unrelated caller - the `jal` that now points at the
wrong one.

Read the addresses out of `configs/JP1/sym.<target>.txt` before writing a new
unit, and link the one target afterwards:

    make -j16 build/JP1/out/BTLP.BIN
    sha256sum --ignore-missing --check configs/JP1/checksum.sha

## An abs written as a ternary keeps the delay slot empty

`if (n < 0) { n = -n; }` and `n = n < 0 ? -n : n;` are the same three
instructions and different delay slots. Written as the `if`, gcc fills the
`bgez` by stealing the first instruction of the block the branch lands in -
redirecting the branch past it - and the routine comes out one word short of
the image, which has a `nop` there. Written as the ternary it does not, and the
block after the join starts where the image starts it.

    gap = o->col2 - col;
    gap = gap < 0 ? -gap : gap;      /* the image's shape           */
    if (gap < 0) { gap = -gap; }     /* one word short, every time  */

- [aimorder.c](/src/btlp/aimorder.c) - both walks went from 94% to 99% on that
  line alone, and nothing else moved.

Worth trying on any short conditional whose branch the build fills and the
image does not.

## A temp assigned inside the arm is not propagated into it

Where the image clamps into a second variable, the copy has to be *inside* the
arm that goes on to test it. Written above the `if`, gcc propagates the copy
away and the test comes out on the original:

    if (odds >= 0) {
        chance = odds;                       /* here: the test is on chance */
        if (chance > MAX) { chance = MAX; }
    } else {
        chance = 0;
    }

    chance = odds;                           /* above: the test folds back
    if (odds >= 0) {                            onto odds and one word differs */
        if (chance > MAX) { chance = MAX; }
    } else {
        chance = 0;
    }

The copy still lands in the branch's delay slot either way, so the diff is a
single `slti` on the wrong register. Read which register the image compares
and put the assignment on that side of the brace.

- [defeatdrop.c:64](/src/btlp/defeatdrop.c#L64) - exact with the copy inside
  the arm, one word out with it above. The `else` arm being the negative case
  is the branch-polarity rule in the section above it.

## Walk the id, not the index

Two tables indexed by the same running value - `t[first + i]` and
`u[first + i]` - are not strength-reduced by gcc 2.6: it works `first + i` out
afresh every turn and multiplies twice. Step the *id* instead and both
disappear into induction variables, one per table, incremented at the bottom
of the loop exactly as the image has them:

    do { ... t[id] ... u[id] ... id++; i++; } while (i < n);

- [spelllines.c](/src/btlp/spelllines.c) - forty-eight bytes over as
  `spell + i`, exact as `spell++`.

## Index a record array rather than walking it

A `BtlActor *a` walked with `a++` and read at three offsets gets biased to the
largest of them, so the record's own offsets come out negative and the
preheader carries an extra `addiu` for the bias. `g_btl_combatants[i]` with a
plain counter keeps the base where the image has it and the offsets positive.

- [aimorder.c](/src/btlp/aimorder.c) - `a[i].c.key` against `a->c.key`, three
  instructions and the whole allocation.

## A field the image reads twice, and gcc reads once

Where the image loads the same field again a few instructions later, no
ordinary spelling gets it back: `a->move` written out four times is CSEd into
one load whether the reads are in one expression, in separate statements, or
either side of a branch, and a second pointer to the same record is copy-
propagated away before CSE ever runs. Reading through a volatile lvalue is the
only thing that separates them:

    #define BTL_MOVE(a) (*(volatile u_char *)&(a)->move)

Nothing can change the byte in between, so this says only "do not keep it" -
but it is load-bearing, and each read has to go where the image puts one.
Count the `lbu`s in the image, decide which tests share each of them, and put
a plain read where the image shares and a volatile one where it does not.

- [aimmove.c](/src/btlp/aimmove.c) - four instructions short and 95% with
  plain reads, exact with three of the eleven made volatile. Holding the value
  that is shared in a `u_char` local rather than an `int` is the other half of
  it: an `int` adds an `andi` the image has no room for.

## A global read at the top of every turn, not above the loop

A global the loop body assigns from is hoisted into the preheader unless it is
taken into a local *inside* the body. Where the image re-reads it each turn,
the tell is the order of the stores around it rather than the load itself:
hoisted, the load leaves the block early and the stores after it come out in
the wrong order.

    do {
        gfx = g_btl_unused_gfx;     /* here, not above the do */
        p->kind = index;
        p->motion = FX_MOTION;
        p->scripts = gfx;
        p = p->attached;
    } while (p != 0);

- [movefx.c:86](/src/btlp/movefx.c#L86) - 96.50% to 98.21% on that line.

## The byte offset a loop walks a record table by can be the source's own

`g_btl_actors[slot]` read at four offsets makes gcc derive a byte offset and
step it 0xEC a turn - the same induction variable the image has. What differs
is *where it is set up*: a compiler-made one is emitted last in the preheader,
after every statement the source puts in front of the loop, and no order of
those statements moves it. Where the image sets the offset up before one of
them, the offset is the source's own variable:

    off = BTL_PARTY * sizeof(BtlActor);
    p   = g_btl_pick_slots;         /* the image sets this up after off */
    do {
        if (ACTOR_FIELD(u_char, off, c.key) != 0) { ... }
        slot++;
        off += sizeof(BtlActor);
    } while (slot < BTL_PARTY + BTL_ENEMIES);

with one macro keeping the field names:

    #define ACTOR_FIELD(type, off, field)         (*(type *)((char *)g_btl_actors + (off) + (int)&((BtlActor *)0)->field))

The generated loop body is identical either way - `lui %hi(sym+field)`,
`addu at,at,off`, load - so this costs nothing but the spelling.

- [pickrandom.c](/src/btlp/pickrandom.c) - both pickers 97% with the slot
  indexed and exact with the offset spelt out. Twenty-four orderings of the
  four set-up statements and a permuter run had not moved it.

Read it off the preheader: a set-up instruction that comes *between* two the
source clearly owns is not the compiler's.

## Walk the id when two tables are indexed by it

Covered above for spell lines; the same thing decides
[actordef.c](/src/btlp/actordef.c)'s sibling in reverse - there the record is
read through one pointer taken once, because only one table is indexed.

## Reuse the local, do not give the value one of its own

Section 4 goes both ways, and the reuse direction closed two routines in the
effect block that no ordering had moved.

**Two addresses, one local.** A routine that reads the loader's first address,
uses it, then reads its second wants *one* variable taken twice rather than two
expressions:

    stage = D_80140004;                 /* the artwork  */
    memcpy(dst, stage, FX_GFX_BYTES);
    stage = g_load_stage;               /* and the tim  */
    BtlUploadTim((u_long *)stage, ...);

Written as two expressions in place, gcc hands the block move the load itself;
written this way it hands it a copy, which is the instruction the image has.

- [movefx.c:80](/src/btlp/movefx.c#L80) - 98.21% to exact on those two lines.

**The call's answer goes in the local that already held something else.** A
loop that reads a fighter's object at the top and makes a record at the bottom
keeps *one* pointer for both, not one each. With a variable of its own, gcc
builds the record's attribute word before it has moved the call's answer out of
`v0`, and the constant and the object come out in each other's registers.

    ob = g_btl_actors[slot].obj;        /* on the way in  */
    ...
    ob = BtlObjAlloc(...);              /* and on the way out */
    ob->attr = FX_LAYER_ATTR;

- [fxlayers.c:84](/src/btlp/fxlayers.c#L84) - 95.75% to exact; the permuter
  found it after a dozen orderings of the three stores had not.

Both are worth trying whenever two values with disjoint lifetimes sit in one
routine and the diff is registers rather than instructions.

## The loop counter is not a temp, whatever section 4 says

The reuse direction has a limit, and it is the counter a loop below is about to
start from. `BtlMemberMotion06` sets a message timer through the same `i` its
dim loop then walks, and the loop pays for it: gcc no longer knows `i` is zero
at the loop's head, so instead of `s2 = 0` and a walking pointer taken straight
off the symbol it emits the whole `i * 0xEC` again and builds all three
induction variables from it - ten instructions for one shared local.

    if (g_btl_msg_speed == 0) { speed = 0xB4; } else { speed = 0x1E; }
    g_btl_msg_timer = speed;            /* not `i` */
    ...
    i = 0;
    do { ... g_btl_actors[i] ... } while (i < BTL_PARTY);

- [memberact.c](/src/btlp/memberact.c) - 91.5% to 93.4% on that one rename.

The sign of it in `odiff --args` is a run of `sll`/`subu` pairs marked **extra**
immediately before a loop, against a `addu sN, zero, zero` in the image.

## The phase is written in every arm, not through one variable

A tick routine whose arms nearly all end `o->phase++` looks like it wants a
`phase` local and one store after the switch. It does not. Written that way gcc
cross-jumps the *load and the add* as well as the store, and four or five arms
come out as a bare `j` into a shared block; the image has `lbu`, `addiu` and a
`j` in each arm and shares only the `sb`.

    case 4:
        ...
        o->phase++;      /* in each arm */
        break;

- [memberact.c](/src/btlp/memberact.c) - 93.4% to 95.1%, seven arms.

## The same row formed in front of one guard and inside the next

`&g_btl_member_scripts[kind * 0x28 + entry]` is a row pointer the image builds
in two different places in the same routine - above the `if` that uses it in
one arm and inside it in the next - and the diff says which. Above the guard
the address computation sits before the `and`/`bnez` of the flag test; inside
it, after. Do not make the two arms agree with each other; make each agree with
the image.

- [memberact.c](/src/btlp/memberact.c) - case 0 above, case 3 and case 8
  inside; 1.5% between them.

## Write the whole thing in both arms and let the tail be shared

Section "Two arms that differ only in a constant share their tail" says to lift
the common part out of the arms. The opposite is also a real shape, and it is
the one every start handler that chooses a side uses: the *whole* group of
statements is written in both arms and gcc's cross-jumping folds the identical
tail back into one copy at the label the arms meet at.

    if (g_btl_actor_turn < BTL_PARTY) {
        pos[0] = 0; pos[1] = -FX_52_OFF; pos[2] = 0;
    } else {
        pos[0] = 0; pos[1] =  FX_52_OFF; pos[2] = 0;
    }

Lifted out behind one chosen offset, gcc turns the arms into "load the else
value, branch over the then value" and the three stores come out *ahead* of the
test. Written out twice, the arms are one `lui` each and the three stores land
at the join - which is what the image has, `j` and all.

- [fxspell52.c](/src/btlp/fxspell52.c) - 76% to exact on that alone.
- [fxspell1b.c](/src/btlp/fxspell1b.c) - `BtlFxStart1C`, the same for the two
  position words plus `pos[2]`; 93.8% to 95.9%.

Read the image first: the giveaway is a `j` between the arms. No `j` means the
value was lifted out, and section "Two arms..." applies instead.

## Let strength reduction produce the running offset

A row of records laid out in depth or across the field reads naturally as a
running value stepped once per turn:

    z = -FX_56_BACK;
    ...  o->z = z;  z += FX_56_GAP;

That is a variable, and it costs a register the image does not spend - the
constant gets hoisted into a saved register too, so it costs two. Written as
the product the row actually is,

    o->z = -i * FX_56_GAP;

gcc's strength reduction builds exactly the same running add, initialises it to
`-2 * GAP` and *re-materialises* the step with a `lui` inside the loop, which is
the image's register count and the image's instruction.

- [fxspell56.c](/src/btlp/fxspell56.c) - 71% to 94% on that line.
- [fxspell2f.c](/src/btlp/fxspell2f.c) - `(i - 1) * FX_2F_SPREAD` for the same
  reason.

## One local threaded through a whole tail

Section "Reuse the local" again, at the scale of a whole run of statements. A
routine that finishes by copying three shorts into a record and then writing
four more out to globals has, in the image, *one* register carrying every one
of those values in turn. Give each value a name and the loads become
independent, the scheduler interleaves them, and the tail comes out shuffled -
a dozen words out of place with nothing structurally wrong.

    n = g_btl_cam_rot.vx;  o->rot.vx = n;
    n = g_btl_cam_rot.vy;  o->rot.vy = n;
    n = g_btl_intro_dist;  o->rot.vz = n;
    n = FX_6E_LIT;         g_btl_scene_rgb[1] = n; g_btl_scene_rgb[2] = n;
                           g_btl_scene_rgb[0] = 0;
    n = FX_6E_FADE;        g_btl_arena_fade = n;

gcc 2.6 gives a local one pseudo for the whole function, so reusing it puts a
true anti-dependency between every pair and the scheduler cannot reorder them.

- [fxspell6e.c](/src/btlp/fxspell6e.c) - 81.8% to exact; the intermediate step
  of reusing one local for only the three angles took it to 86%.

## A pointer to the field, and the record worked out from it

Where a loop rewrites one field of a global template and then hands the
template to a call, the image keeps *the field's* address in the saved register
and works the template's out from it - `addiu a0, s8, -0x4` - rather than
naming the symbol a second time.

    scripts = &g_btl_fx_def.scripts;
    ...
    *scripts = ...;
    o = BtlObjAlloc((BtlObjDef *)(scripts - 1), ...);

Three shapes and none of the other two is it: `&g_btl_fx_def` at the call site
gets its own `lui`/`addiu` and no CSE; a `BtlObjDef *` local biases the register
to the record instead of the field, so the store gains an offset and the call
loses one. The tell is which of the two the store's displacement is zero on.

Assigning the pointer before the loop also matters for a second reason - see
"A constant kept in a local of its own": a hoisted invariant lands in the
preheader, *after* everything the source puts in front of the loop, so a
register set up ahead of the loop's other locals cannot be one gcc lifted.

- [fxspell15.c](/src/btlp/fxspell15.c) - 97.8% to exact.

## Step the loop counter where the image steps it

`for (; col < W; col++)` puts the increment last, after every other statement
in the body. When the image has it early - filling a divide's latency, say, or
sitting between two stores - the source stepped it by hand:

    for (; col < FX_GRID_W; ) {
        ...
        col++;          /* where the image has it */
        ...
    }

The tell is two counters swapping places in the diff: the scheduler fills the
same two slots either way and takes whichever came first in the source.

- [fxspell1b.c](/src/btlp/fxspell1b.c) - `BtlFxStart1C`, `col++` between the
  record's link and its attribute.
- [fxspell15.c](/src/btlp/fxspell15.c) - `col++` and `cell--` both *before* the
  allocation, which no amount of reordering at the end of the body can produce:
  the scheduler will not move them across the call.

## Force the association with a temp when the arms share a term

Section "Two arms that differ only in a constant share their tail" tells you to
compute the differing part in the arms. The other half of the same lever is to
compute the *shared* part in front of them, and it is worth reaching for the
moment the diff shows a table reading duplicated into both arms:

    wide = g_btl_wave_cos[o->angle] * FX_2A_WIDE;
    o->y = (g_btl_actor_turn < BTL_PARTY) ? wide - FX_2A_LIFT
                                          : wide + FX_2A_LIFT;

- [fxspell2a.c](/src/btlp/fxspell2a.c) - `BtlFxStep2A`, 85.07% to 98.41%.
- [fxspell27.c](/src/btlp/fxspell27.c) - the same for the row a spark stands
  on, 85.97% to 92.40%.

And where the residual is which of two additions happens first, a temp forces
it. `o->scale_x + 8 + o->scale_x / 8` is reassociated by gcc into
`(scale/8 + 8) + scale`; written

    grew = o->scale_x + FX_2A_GROW;
    o->scale_x = grew + o->scale_x / FX_2A_EIGHTH;

it is the image's `(scale + 8) + scale/8`, and that was the last word of
`BtlFxStep2A`.

## The load's width says how the source read the field

A byte load where the field is an `int` is not the compiler narrowing for you -
it is the source reading it as a byte. `g_btl_spell_fx[...].group` is an `int`
and the image reads it with `lbu`, so the cast is in the source:

    o->children = (u_char)g_btl_spell_fx[g_btl_fx_move].group;

The signedness reads the same way: `lb` against a `u_char` field means this
translation unit read it as a `signed char`, the way `lhu` against a `short`
means unsigned. `BtlFxStep2F` reads `Char.status` with `lb` where char.h has it
unsigned, and casting was the whole difference on that instruction.

## A magic multiply's post-shift is part of the divisor

`mult x, 0x2AAAAAAB; mfhi; sra 2` is not a divide by six with a stray shift -
it is a divide by twenty-four. Read the shift as part of the constant before
believing the multiply: `mulhi(x, M)` gives `x / 6` and the `sra 2` finishes
the job. Writing the six compiles, links, and is wrong by a factor of four.

- [fxspelle5.c](/src/btlp/fxspelle5.c) - the step a fleeing enemy is given is
  its distance over the twenty-four frames it has, which is also why the two
  constants are the same number.

The tell that a divisor is *not* what it looks: gcc emits no post-shift for the
plain case, so a `sra` between the `mfhi` and the sign fixup always means the
divisor is larger than the multiply alone says.

## Two constants that happen to be equal must not share

`g_btl_clut_fading |= 1 << g_btl_hit_slot;` and `i = 1;` next to each other
give gcc one register holding 1 and two uses of it; the image materialises the
one twice. Putting the shift *before* the counter's initialiser is enough - the
counter is then no longer live when the shift needs its own.

- [fxstep86.c](/src/btlp/fxstep86.c) - 89.94% to exact with that and the line
  below.

## Which of two stores is written first decides how the scheduler pairs them

Where two fields of the same record are set back to back and the diff has them
in each other's slots, the source order is the lever and it is not always the
image's store order - the scheduler reverses a pair as readily as it keeps one.

    n->kind = o->kind;          /* written first  */
    n->mark_num = FX_COPY_MARK; /* stored first   */

- [fxstep09.c](/src/btlp/fxstep09.c) - 99.65% to exact; a local for the kind
  and either store order got the load hoisted but left the registers swapped,
  and only writing the kind first fixed both.
- [fxstep86.c](/src/btlp/fxstep86.c) - the target's own word cleared before its
  record is reached for, though the image stores it after the record's load.
- [fxspell42.c](/src/btlp/fxspell42.c) - `BtlFxStart43`: the first record's
  attribute stored before the template is rewritten for the second record.
  The other way round, the template's address and the 0xE argument also trade
  saved registers; 96.21% to exact.

## One routine can want both arm shapes

The same expression twice in one routine does not want the same treatment
twice. `BtlFxStep09` sets its record's step from the acting side in two places:
the first is an assignment, and the image writes the store out in both arms; the
second is a `+=`, and the image loads the field once and preloads the else
value. Written the same way both times it is wrong in one of them, whichever
way round you choose.

Read each occurrence's own branch in the image - a `j` between the arms means
write them out, no `j` means the ternary.

## A local of the global's own width keeps the load and the copy apart

Where the image loads a global, tests the fresh load, and puts a copy into
another register in the branch's delay slot for the arithmetic after, the local
wants the global's width. An `int` local takes the load straight into its own
register and the delay slot comes out empty; a `short` local of a `short`
global leaves the sign-extended load for the tests and the narrow copy for the
sums.

- [menunav.c](/src/btlp/menunav.c) - `BtlSpellMenuUpdate`, 98.88% to exact on
  `short row`.

It does not carry over to a byte field read through a `(signed char)` cast:
`signed char`, `short` and `int` locals all leave
[pickother.c](/src/btlp/pickother.c) one instruction out.

## Index a table in two dimensions when the base goes in before the second index

`g_btl_reach[lane * 15 + row]`, `g_btl_reach[row + lane * 15]` and
`(g_btl_reach + lane * 15)[row]` all compile to the same code, adding the base
last. Declared `[5][15]` and indexed `[lane][row]`, the base goes onto the
scaled first index and the second is added to that, which is the image's order.

- [reach.c](/src/btlp/reach.c) - `BtlMarkMoveArea`, 97.21% to exact.
- The same goes for a row read in a loop: `g_btl_hit_rolls[hits][i]` inside
  the loop, not `row = g_btl_hit_rolls[hits]` taken first, which works the
  row out ahead of the table's address. [hitroll.c](/src/btlp/hitroll.c) -
  `BtlRollHits`, 94.59% to exact.

## A row read inside a loop is reached through pointer arithmetic on the table

When the image loads a loop's compare constant (`ori a1, zero, 0xff`) *before*
it builds the row's address (`lui`/`addiu` of the table, then the scaled row),
the row was an address worked out inside the loop and strength-reduced after
invariant motion. `(table + slot)->cell[i]` does that; `table[slot].cell[i]`,
`(table + slot * 25)[i]`, a row pointer taken before the loop, or indexing the
cells as one long run all fold the row into the symbol's address and build it
ahead of the constant.

- [formationpreset.c](/src/btlp/formationpreset.c) - `BtlFormationPresetEmpty`
  57.89% and `BtlFormationPresetFits` 86.21%, both to exact.
- [placecursor.c](/src/btlp/placecursor.c) - `BtlStandPreset`, guarded at
  96.31%, to exact; `BtlPlacePreset` 91.95% to 98.52%.

## A byte local: a copy before the int tests, and a frame in a leaf

A status read with `lb` into one register and copied into another
(`addu v1, a1, zero`) before a range test, in a leaf that sets up an 8-byte
frame it never uses, is a `signed char` local. The int comparisons work on a
widened copy of the byte pseudo, and the pseudo's slot is the frame. An `int`
local with the cast on the load gives neither; a spare array gives the frame
but not the copy.

- [enemy2.c](/src/btlp/enemy2.c) - `BtlBattleOutcome`, 93.61% to exact.

## Initialise the counter in the `for`, not ahead of the test before the loop

A loop whose entry test is `blez` on the bound, with the counter zeroed in a
delay slot above it, has the zero in its own `for (i = 0; ...)`. Zeroing the
counter ahead of an `if` that comes before the loop and writing `for (; ...)`
gives a `slt`/`beqz` entry test instead - and, in `BtlBoardOpen`, an 8-byte
smaller frame.

- [boardopen.c](/src/btlp/boardopen.c) - `BtlBoardOpen`, 96.91% to exact.

## A list packed as it is filled is indexed by a count, not walked

When a loop copies into a destination only for the entries that qualify, and
the image's destination pointer is set up *after* the loop's hoisted constants
and the strength-reduced offset of its other arrays, the source indexed the
destination with a count it bumps on each copy - `memcpy(lines[n], ...); n++`.
gcc reduces that index to the pointer the image steps, and a reduced
variable's start is inserted after invariant motion. A pointer assigned before
the loop (`line = lines[1]; ... line += 11`) keeps its set-up ahead of the
constants, wherever the assignment is written.

- [stockboard.c](/src/btlp/stockboard.c) - `BtlOpenStockBoard`, 97.15% to
  exact on both of its packed lists.

## Rows a loop touches at several fixed distances are indexed, not walked

A loop that writes a row and the rows ten, twelve and twenty-two after it
reads, in the image, as one pointer based on the *first* row with the others
at fixed offsets (`addu a0, s1, zero`, then `0x82(a0)`, `0x9A(a0)`). A walked
pointer (`c = rows; ... c[10].h ... c++`) leaves gcc free to base its reduced
pointer on another row - here the last one, `addiu a0, s1, 0x112`. Writing
`rows[i].h`, `rows[i + 10].h` and so on gives it one reduced address to hang
all of them from, and it takes the first.

- [objmotion6.c](/src/btlp/objmotion6.c) - `BtlObjMotion06`, 98.96% to exact,
  and `BtlObjMotion07`, 97.48% to 99.98%.

## A byte stepped down is read signed when the image adds a negative

`field -= 3` on an unsigned byte folds the step into the byte's own width and
the image shows `addiu v1, v1, 0xfd`; where it shows `addiu v1, v1, -0x3` the
byte was read signed first: `field = (signed char)field - 3`. The field can
stay unsigned in the struct - every other reader of it loads the same bits.

- [objmotion6.c](/src/btlp/objmotion6.c) - `BtlObjMotion07`, 99.98% to exact,
  on the two rows that step the font row back.

## A field tested after stores through a byte is read into a local before them

Where the image loads a field early, then makes stores through a byte field of
some other record, and only then subtracts from the value it loaded, the
source read the field into a local before those stores. Written straight into
the test, the field is read again after them - a store through a byte may
alias anything, so gcc cannot keep the earlier load.

In the same routine a clamp the image does on a copy of the value (`bltz a0`
with `a1 = a0` in its delay slot) is a ternary, not an if chain, and two
updates in a row the image stores `+0x18` then `+0x10` were written
`+0x10` then `+0x18`: the scheduler turns an independent pair round.

- [fxspell87.c](/src/btlp/fxspell87.c) - `BtlFxStep87`: 86.94% as first
  written, 90.52% with the ternary clamp, 96.77% with the experience read into
  a local ahead of the hit stores, and exact with the last two updates swapped.

## The table read written first keeps its address in a register

A loop that multiplies a table entry by a call's result -
`(rand() % 16) * g_btl_wave_cos[i * 0x38]` - with the call written first has
the table's address folded into the load (`lui at` / `addu at, at, s2` /
`lw v1, %lo(g_btl_wave_cos)(at)`). Where the image takes the address into a
register ahead of the arithmetic on the call's result (`lui v0` / `addiu v0` /
`addu v1, s2, v0` / `lw v1, 0x0(v1)`), and the remainder's bias moves to a
register of its own because of it, the table read was the first operand:
`g_btl_wave_cos[i * 0x38] * (rand() % 16)`. A local for the call's remainder,
a local for the angle, or a pointer into the table all leave it further off.

- [fxspell39.c](/src/btlp/fxspell39.c) - `BtlOpenFxScatter`: 94.61% to exact.

## A store both arms end with is written in both arms

Where the image makes a store just after an if/else joins but ahead of the
next call's argument set-up - `sw zero, 0x28(sp)` before
`lui a0, %hi(g_btl_fx_def)` - the source ended both arms with it, and gcc
merged the two identical tails into the one store at the head of the join.
Written once after the if, it is expanded together with the call and the
scheduler puts it among the argument stores instead.

- [fxspell38.c](/src/btlp/fxspell38.c) - `BtlFxStep38`: 94.91% as first
  written, 98.80% with the cell's scripts copied ahead of its mark (which also
  pulls the running x step up among the copies, where the image has it), and
  exact with `pos[2] = 0` written in both arms.

## A copy written ahead of a test, where the image has it in the branch's delay slot

Where two locals that live through a whole loop nest trade saved registers and
everything else matches, look at the plain copies around a test. `after = o`
written after `if (o->mark_num == FX_MARK_HEAD) head = o;` gives `after` a
different place in the allocator's order from the image's; written ahead of
the test - which is where the image has it, in the delay slot of the test's
branch - `after` and the hoisted copy of the column's x come out in the
image's registers. Declaration order and where `after` is first set did not
move them.

- [fxsheet.c](/src/btlp/fxsheet.c) - `BtlFxStart05`: 99.67% to exact, a swap
  of s2 and s3 with the caller-saved spills and stack slots already right.

## Take the row of a two-dimensional table first when its base and the column trade registers

Where the image forms `table + SCRIPT + key * 40` in one register and then adds
`pick * 10` to it, with the table's address and the pick in each other's
argument registers, the source took the row as a pointer and indexed the
column off it:

    row = &g_btl_member_scripts[SCRIPT_RISE + key * MEMBER_SCRIPT_MODEL];
    script = row[o->actor->script_pick * MEMBER_SCRIPT_PICK];

All six orders of the three terms summed into one index, the sum taken into an
`int` or a `u_char` local, and `(table + SCRIPT)[...]` leave the two registers
swapped or worse.

- [fxspell42.c](/src/btlp/fxspell42.c) - `BtlFxStart42`: 99.47% to exact.

## Reuse the first loop's locals in the second

When a routine's second loop needs a lane and a row of its own, the image may
be keeping them in the registers the first loop's `left` and `first` had. Using
those same locals keeps `left` alive past the first loop, and that is what lets
cse test the hoisted `left` instead of the loop's copy - the image's `slti` on
the lane moves out of the loop.

- [reach.c](/src/btlp/reach.c) - `BtlMarkEnemiesAround` 91.97% and
  `BtlMarkPartyAround` 91.67%, both to exact.

## A cast on the call keeps the zero-extension

`keys = (u_short)BtlMenuKey();` into an `int` gives the image's
`andi s0, v0, 0xffff`. A `u_short keys` local is folded away, because every
later use only tests bits.

- [menunav.c](/src/btlp/menunav.c) - `BtlPresetMenuUpdate`, 99.55% to exact.

## Return the assignment

An `addu v0, s0, zero` just before a global is stored from `s0` is the routine
answering what it stored: `return g_btl_edit_board = o;`.

- [editboard.c](/src/btlp/editboard.c) - `BtlOpenEditBoard`, 98.82% to exact.

## Work in ints before narrowing

A width written into a `u_short` field and then read back for the next
position gets the arithmetic done on the field's type - `ori 0xffc0` and an add
- where the image has `addiu -0x40`. Keep each width in an `int` local, store
it, and build the positions from the locals.

- [editboard.c](/src/btlp/editboard.c) - `BtlFillEditBoard`, which also needed
  its digit cells as separate symbols; 94.44% to exact.

## Arms that end the same way share their tail through a goto

Two arms that end in the same call and return come out as one cross-jumped
tail in the image. Written with that tail in each arm gcc keeps both copies;
written `if ... else if ... else goto past;` with the tail once after them, it
emits the one.

- [menunav.c](/src/btlp/menunav.c) - `BtlSpellMenuUpdate`, 93.51% to 98.88%.

## A global kept across a call is reached through a local pointer

When the image loads a global's address into a saved register, stores through
it in a call's delay slot and reads it back through the same register after,
the source took the address into a local: `slot = &g_btl_talk_board_slot;`
and then `*slot = ...` and `*slot` around the calls. Written against the
global directly - or as an assignment inside the argument - gcc reaches it with
a fresh `lui` each time and the saved register never appears.

- [stocklist.c](/src/btlp/stocklist.c) - `BtlRunTalkBoard`, 83.28% to exact.

## A range test on the field itself keeps a copy of the load

Where the image loads a value, tests the fresh load, and then copies it into a
second register for a `(u_int)(x - lo) < n` range test,

    lb    v0, %lo(g_btl_actors+0x49)(at)
    beq   v0, s3, skip
    addu  v1, v0, zero          # the copy
    addiu v0, v1, -0x10
    sltiu v0, v0, 2

the source compared the field (or global) directly each time and let gcc fold
the adjacent pair into the range itself:

    if ((signed char)g_btl_actors[n].c.status != BTL_STATUS_POISON
        && (signed char)g_btl_actors[n].c.status != BTL_STATUS_SICK
        && (signed char)g_btl_actors[n].c.status != BTL_STATUS_DOWN) {

Taken into a local first and written as the range - `st = ...status;` and
`(u_int)(st - BTL_STATUS_SICK) >= 2` - the load goes straight into the register
the range test uses and the copy is gone. Nothing done with locals brings it
back: a copy inside the arm, a `short` or `signed char` local, re-reading the
field for the first test only. `==`/`!=` pairs and `>=`/`<=` bounds fold the
same way.

The fold also grows the frame, and nothing reads the extra space: eight bytes
in `BtlStageClose`, sixteen for the two tests in `ovl_btlp_entry`. Both routines
had been padded to the image's frame size with locals that are not there -
`SVECTOR unused`, and an `image[6]` that only ever uses one element - so take
the padding back out when the fold goes in.

- [roundflow.c](/src/btlp/roundflow.c) - `BtlStageClose`, 99.74% to exact, with
  the pad removed.
- [entry.c](/src/btlp/entry.c) - `ovl_btlp_entry`'s encounter tests,
  `g_btl_encounter == 0x11 || g_btl_encounter == 0x12`, and `image[2]` for
  `image[6]`; both copies came back and the frame stayed the image's.

## A constant in a branch's delay slot can come from the fall-through

A variable's first value sitting in the delay slot of the test just before its
loop is not evidence that the source assigned it before the test. reorg fills
the slot with the first instruction on the fall-through side, and that is where
the source had it:

    if (*wide == BTL_SLOT_FREE) {
        found = 0xC;
        goto load;
    }
    {
    u_int slot = BTL_SLOT_FIRST;    /* image: `ori v1,zero,0xA` in the test's slot */

Assigned above the test, `slot` is live across it and cannot share a register
with the value the test compares, so that value moves to another register.

- [enemyload.c](/src/btlp/enemyload.c) - `BtlLoadEnemyGfx`, 99.97% to exact;
  with `slot` set above the test, `*wide` was loaded into `t0` instead of `v1`.

## The scratch script's address is read with the offset spelled out

The talk scenes find a script in the scratch file by reading the directory
offset at its head, indexing the table past it, and adding both back on. The
image loads the table entry into `a0` and adds the base and then the offset to
it in place:

    lw    a0, 0(v0)        # the table entry
    addu  a0, a0, v1       # + the scratch base
    addu  a0, a0, a1       # + the directory offset

With the offset read into a local first - `dir = *(u_long *)BTL_SCRATCH;` and
then `BTL_SCRATCH + dir + table[...]` - gcc still loads it once, but it adds the
entry and the base *to the offset*: `addu a0, a1, a0`. Spelled out at both uses
it comes out as the image:

    BtlSeqPlay(BTL_SCRATCH + *(u_long *)BTL_SCRATCH
               + *(u_long *)(BTL_SCRATCH + *(u_long *)BTL_SCRATCH + *line * 4));

The two reads are CSEd into one load, so nothing is paid for it. What changes is
that the offset is an expression rather than a variable when the sum is built,
and a variable gets associated differently. A cut-down copy compiled on its own
(see [RTL Dumps](RTL%20Dumps.md)) showed the same thing before either routine
was touched.

- [begintalk.c](/src/btlp/begintalk.c) - `BtlBeginTalking`, the last word.
- [talkscenedemand.c](/src/btlp/talkscenedemand.c) - both script plays in
  `BtlTalkSceneDemand`, 97.45% to 98.03% on that alone.

## One saved register doing several jobs is one variable

Where the image keeps a single saved register for values whose lifetimes never
overlap, the source had a single variable. `BtlBeginTalking` uses `i` as a loop
counter, then for the moon test, then for the menu's answer, then for the
line's base address, and `n` counts the live offers and then picks the opening
line. Given variables of their own, the short-lived ones land in `v1` or get
tied to their neighbour, and the copies the image makes between them vanish.

Where the image copies one of those into another register *after* a call - `addu
s2, s0, zero` in a branch's delay slot - the copy is the else arm of an `if`,
not an assignment above it:

    i = g_btl_moon != MOON_FULL;
    if (rand() % TALK_OPEN_SETS == 0) {
        n = TALK_OPEN_SPARE;
    } else {
        n = i;
    }

- [begintalk.c](/src/btlp/begintalk.c) - `BtlBeginTalking`, 99.70% to 99.94% on
  the reuse and the else arm together.

## A call that ends its basic block keeps its arguments in expand order

gcc expands a call as the stack arguments first and then the register
arguments, and its `sched` pass normally reorders that - the image's calls load
`a0`..`a3` and store the fifth argument last, often in the call's delay slot.
sched does not touch a call that is the **last insn of its basic block**, so
that one comes out stack argument first:

    ori v0,zero,0xC ; sw v0,0x10(sp) ; ori a0.. ; ori a1.. ; ... ; jal   # last in block
    ori a0.. ; ori a1.. ; ... ; ori v0,zero,0xC ; jal ; sw v0,0x10(sp)   # scheduled

Where the image's order is the scheduled one but the source ends a block at the
call - two arms that each make a call and then fall into shared code - write the
shared code into each arm. jump2's cross-jumping runs after sched and folds the
copies back into one tail, so the image still shows a single copy, and a `j`
from the first arm straight to the second arm's `jal`.

    if (...) {
        BtlOpenMessage(1, 1, g_btl_move_lines[a->move], 0x10, stop);
        if (g_btl_msg_speed == 0) { speed = 0xB4; } else { speed = 0x1E; }
        g_btl_msg_timer = speed;
    } else if (...) {
        BtlOpenMessage(1, 1, D_800CF7EC, 0x10, CAST_AIL_0C);
        if (g_btl_msg_speed == 0) { speed = 0xB4; } else { speed = 0x1E; }
        g_btl_msg_timer = speed;
    }

The `.combine` and `.sched` dumps show it directly - see
[RTL Dumps](RTL%20Dumps.md) for getting and reading them.

- [memberact.c](/src/btlp/memberact.c) - `BtlMemberMotion06`, 98.61% to exact
  with this, `(signed char)` casts in place of `*(signed char *)&` on the
  acting fighter's status, and a local for `o->actor` across a byte store.

## A store the image makes between the join's two stores goes last in both arms

Where both arms of a side test set `pos[0]` and `pos[1]` and the image's join
stores `pos[0]`, then `pos[2]`, then `pos[1]`, the source wrote `pos[2]` inside
the arms as well, last. Written once after the `if`, its store is scheduled
down among the next call's argument set-up instead.

    if (g_btl_actor_turn < BTL_PARTY) {
        pos[0] = ...; pos[1] = ...; pos[2] = layer * FX_12_RISE * PLACE_FIXED;
    } else {
        pos[0] = ...; pos[1] = ...; pos[2] = layer * FX_12_RISE * PLACE_FIXED;
    }

- [fxspell12.c](/src/btlp/fxspell12.c) - `BtlFxStart12`, 90.88% to exact with
  this, the depth as a product rather than a stepped `z`, `after` cleared
  before `row` is set, and the ring's three stores written whole in both arms.
- [fxspell1e.c](/src/btlp/fxspell1e.c) - `BtlFxStart1E`, 91.98% to exact with
  this, the same product, and the first loop's `cell` reused as the second
  loop's mark counter.

## A call whose argument loads come first is written first

`BtlFxStart1E`'s image loads `BtlTintTargets`' four arguments ahead of the
saved-register inits for `cell` and `after`, with `row = 0` in the call's
delay slot. Every order of the three inits in front of the call scores the
same; moving the call ahead of them all is exact. Read the argument loads'
position against the inits before permuting the inits.

- [fxspell1e.c](/src/btlp/fxspell1e.c) - 92.97% to 96.82% on that alone.
