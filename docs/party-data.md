# The party, and what it carries

Everything here lives in the save-game work area at `0x801Fxxxx`, which is why
DNG, ADV and S2D all reach it at the same address and one source usually covers
all three. The layouts themselves are in `include/persona/common/char.h` and
`include/persona/common/persona.h`; this is how the pieces refer to each other.

## The chain

    g_party[5]            slot -> character index, 0xFF for an empty slot
      -> g_chars[]        0x60 bytes, five of them, at 0x801F1BCC
           .list[3]       -> g_personas[], 0xFF for an empty slot
                -> g_personas[]    0x40 bytes, at 0x801F1DAC, right after
                                   the character records
                     -> g_persona_data[]   0x38 bytes, in main's rodata at
                                           0x80041760

A party slot is not a character: `g_party[slot]` indexes the character records,
and almost every routine that takes a slot starts by resolving it. `PartyLastSlot`
walks down from slot 4 to the first occupied one; each overlay caches the answer
in its own `g_party_last` when it starts up, and the "does anybody..." helpers in
`partyany.c` walk `0..g_party_last` inclusive.

`PartyFindByKey` goes the other way, from the key byte at `Char+0x3E` to a slot.
Event scripts name a character by key rather than by slot, because the slot
changes as the party does.

## Status

`Char+0x49` is an ailment code, 0 for none. Event scripts set it, clear it, and
test it against a script operand; the effect at `0x800816CC` inflicts code 13 on
the party leader. Recovery items go the other way: `ItemUsableOn` maps an item id
onto the question worth asking, so 0x67 asks `CharHasStatus(slot, 13)` and 0x6A
asks for 16, while 0x5F..0x61 restore HP and ask `CharBelowMax(slot, 0)` instead.
The item menu greys out an entry when `PartyAnyStatus` or `PartyAnyBelowMax` says
nobody would benefit.

## Stats

A character carries the five stats twice: `stat[5]` at `+0x4C` and
`stat_base[5]` at `+0x51`. The status screen draws the base as the solid part of
the bar and the difference as a highlight above it, which is what tells the two
apart. Both are clamped at 99 for display.

A Persona has one set, at `+0x26` of its record, and the reference data has its
own at `+0x28`. The three cases are three routines - `DrawCharStatBar`,
`DrawPersonaStatBar` and `DrawPersonaDataStatBar` - drawing into the same five
cell runs, so only one of them is on screen at a time.

The bar itself is a run of `GsCELL`s four units wide, out of a fifteen-glyph
strip at `v = 0x84`:

| u | cell |
|---|---|
| 0x00 | four units of base |
| 0x08, 0x10, 0x18 | one, two, three units of base |
| 0x20, 0x28, 0x30 | one of base plus one, two, three of highlight |
| 0x38, 0x40 | two of base plus one, two |
| 0x48 | three of base plus one |
| 0x50 | four of highlight |
| 0x58, 0x60, 0x68 | one, two, three of highlight |
| 0x70 | four of highlight, where the base ended exactly on a cell |

## Persona reference data

`g_persona_data[]` is static, 0x38 bytes an entry:

| offset | field |
|---|---|
| 0x1C | name, ten tile bytes terminated by 0xFF |
| 0x26 | level, drawn as up to two digits |
| 0x27 | arcana, 1-based into a table of six-cell labels |
| 0x28 | the five stats |

The list at `0x801F297C` holds indices into this table, which is what the row
drawn at `0x80076A8C` formats: name, level, arcana and the bars.
