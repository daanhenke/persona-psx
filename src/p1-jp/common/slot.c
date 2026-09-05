/* Persona 1 (JP) - slot record initialisers.
 *
 * Compiled into more than one overlay rather than called across the boundary:
 *   DNG @ 0x800757EC / 0x80075914 / 0x80075A40
 *   ADV @ 0x80065D60 / 0x80065E88 / 0x80065FB4
 * S2D has the same three at 0x80065850 / 0x80065978 / 0x80065AA4 but against its
 * own table,
 * so it keeps its own copy in src/p1-jp/s2d/slot.c.
 */
#include <types.h>

/* 0x44-byte records at 0x800DC10C, indexed by a u8 slot. A literal address,
   not a linker symbol: the indexed store assembles to `addu $at,rX,$at`, which
   is the form the assembler uses for a numeric base. */
typedef struct {
    /* 0x00 */ int    handle;
    /* 0x04 */ int    unk04;
    /* 0x08 */ int    kind;
    /* 0x0C */ int    unk0C;
    /* 0x10 */ int    unk10;
    /* 0x14 */ short  x;
    /* 0x16 */ short  y;
    /* 0x18 */ short  unk18;
    /* 0x1A */ short  unk1A;
    /* 0x1C */ short  unk1C;
    /* 0x1E */ short  unk1E;
    /* 0x20 */ short  unk20;
    /* 0x22 */ short  unk22;
    /* 0x24 */ short  unk24;
    /* 0x26 */ short  unk26;
    /* 0x28 */ short  scale_x;   /* 0x1000 = 1.0 in 1.12 fixed point */
    /* 0x2A */ short  scale_y;
    /* 0x2C */ short  unk2C;
    /* 0x2E */ u_char unk2E;
    /* 0x2F */ u_char unk2F;
    /* 0x30 */ u_char unk30;
    /* 0x31 */ u_char pad31;
    /* 0x32 */ u_char active;
    /* 0x33 */ u_char pad33;
    /* 0x34 */ int    unk34;
    /* 0x38 */ int    unk38;
    /* 0x3C */ int    unk3C;
    /* 0x40 */ int    unk40;
} Slot;

#define g_slots ((Slot *)0x800DC10C)

/* Fills in one slot: identity scale, cleared everything else, marked active.
 *
 * The three constant stores sit at the end on purpose. gcc hoists them up to
 * where the original has them (right after the y argument is loaded); writing
 * them in their apparent position instead pulls them above the handle store
 * and drags `active` along with them. Do not "tidy" this back into field
 * order - it drops the match to 70%.
 */
void SlotInit(void *def, u_char slot, int kind, short x, short y)
{
    g_slots[slot].handle = (int)def;
    g_slots[slot].unk0C = 0;
    g_slots[slot].kind = kind & 0xFFF;
    g_slots[slot].x = x;
    g_slots[slot].unk18 = 0;
    g_slots[slot].unk1A = 0;
    g_slots[slot].unk1C = 0;
    g_slots[slot].unk1E = 0;
    g_slots[slot].unk2E = 0;
    g_slots[slot].unk2F = 0;
    g_slots[slot].unk30 = 0;
    g_slots[slot].unk20 = 0;
    g_slots[slot].unk22 = 0;
    g_slots[slot].unk10 = 0;
    g_slots[slot].unk24 = 0;
    g_slots[slot].unk26 = 0;
    g_slots[slot].scale_x = 0x1000;
    g_slots[slot].scale_y = 0x1000;
    g_slots[slot].unk2C = 0x80;
    g_slots[slot].active = 1;
    g_slots[slot].y = y;
}

/* Same record, but tags the slot with -2 and puts the definition pointer one
   word later - so +0x00 is a discriminator, not always a pointer. */
void SlotInitTagged(void *def, u_char slot, int kind, short x, short y)
{
    g_slots[slot].handle = -2;
    g_slots[slot].unk04 = (int)def;
    g_slots[slot].kind = kind & 0xFFF;
    g_slots[slot].x = x;
    g_slots[slot].unk18 = 0;
    g_slots[slot].unk1A = 0;
    g_slots[slot].unk1C = 0;
    g_slots[slot].unk1E = 0;
    g_slots[slot].unk2E = 0;
    g_slots[slot].unk2F = 0;
    g_slots[slot].unk30 = 0;
    g_slots[slot].unk20 = 0;
    g_slots[slot].unk22 = 0;
    g_slots[slot].unk10 = 0;
    g_slots[slot].unk24 = 0;
    g_slots[slot].unk26 = 0;
    g_slots[slot].scale_x = 0x1000;
    g_slots[slot].scale_y = 0x1000;
    g_slots[slot].unk2C = 0x80;
    g_slots[slot].active = 1;
    g_slots[slot].y = y;
}

/* Moves a slot and rewrites the low 10 bits of its kind word, keeping bits
   10..27 and dropping the top nibble. */
void SlotSetPos(u_char slot, int kind, short x, short y)
{
    g_slots[slot].x = x;
    g_slots[slot].y = y;
    g_slots[slot].kind = (g_slots[slot].kind & 0x0FFFFC00) + kind;
}

/* Frees a slot: -1 into the handle pair and the four trailing words. */
void SlotClear(u_char slot)
{
    g_slots[slot].unk04 = -1;
    g_slots[slot].handle = -1;
    g_slots[slot].unk34 = -1;
    g_slots[slot].unk38 = -1;
    g_slots[slot].unk3C = -1;
    g_slots[slot].unk40 = -1;
}

/* Sets the seven animation fields in one go. The slot index arrives as a short
   here, not a u_char as everywhere else in this family. */
void SlotSetAnim(short slot, short unk18, short unk1A, u_char unk2E,
                 u_char unk2F, u_char unk30, u_short unk20, u_short unk22)
{
    g_slots[slot].unk18 = unk18;
    g_slots[slot].unk1A = unk1A;
    g_slots[slot].unk2E = unk2E;
    g_slots[slot].unk20 = unk20;
    g_slots[slot].unk22 = unk22;
    g_slots[slot].unk2F = unk2F;
    g_slots[slot].unk30 = unk30;
}
