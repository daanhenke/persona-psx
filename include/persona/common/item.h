#ifndef PERSONA_COMMON_ITEM_H
#define PERSONA_COMMON_ITEM_H

/* Persona 1 (JP) - the item table.
 *
 * 0x20-byte records in the main executable's data, indexed straight by the id
 * an inventory entry carries. Record 0 is the placeholder an empty equipment
 * slot resolves to: its price reads 100,000,000 and its bonus bytes are zero,
 * so an empty slot contributes nothing to a character's numbers.
 *
 * Only the fields the stat routines need are worked out. The two nibble bytes
 * hold one bonus per character stat, and `power` and `rate` are what the two
 * combat values of each equipment group are built from - attack and hit rate
 * for a weapon, defence and evasion for armour.
 */
#include <decomp/types.h>

typedef struct {
    /* 0x00 */ int    price;
    /* 0x04 */ u_short owners;  /* one bit per Char key - who is allowed to
                                   equip this. g_btl_char_bit turns a key into
                                   the bit to test                          */
    /* 0x06 */ u_short unk06;   /* bit 6 marks an entry whose usefulness is
                                   worth asking about: DrawItemRowUsable greys
                                   the row unless it is set and SpellUsable
                                   also says yes. Bits 3 to 5 are which of the
                                   seven equipment groups it belongs to, which
                                   g_btl_equip_kind gives per slot          */
    /* 0x08 */ u_char name[10];   /* packed glyph bytes, ten cells wide */
    /* 0x12 */ u_char pad12[1];
    /* 0x13 */ u_char bonus01;  /* high nibble stat[0], low nibble stat[1] */
    /* 0x14 */ u_char bonus23;  /* high nibble stat[2], low nibble stat[3] */
    /* 0x15 */ u_char bonus4;   /* high nibble stat[4]                     */
    /* 0x16 */ u_char power;
    /* 0x17 */ u_char rate;
    /* 0x18 */ u_char pad18[1];
    /* 0x19 */ u_char area;     /* how wide a swing reaches: BtlMarkMoveArea
                                   takes this and `swing` together to mark the
                                   cells it can land on                     */
    /* 0x1A */ u_char swing;    /* how the swing is aimed. Bit 0, and the whole
                                   byte being 8, both mean it stays on the slot
                                   it was given; bit 1 lets the turn look for
                                   another slot once the mask is walked out;
                                   bit 2 makes a hit free rather than counting
                                   against the number left                  */
    /* 0x1B */ u_char hits;     /* what 0x80094E60 turns into the number of
                                   times the swing lands                    */
    /* 0x1C */ u_char pad1C[4];
} ItemDef;                      /* 0x20 bytes */

extern ItemDef g_item_defs[];

/* The two inventory lists.
 *
 * An entry packs both halves into one u16: the low 9 bits are the item id and
 * the top 7 the count, so a slot counts as in use only when neither half is
 * zero. Both are 0x17F entries and both are reached by hardcoded address
 * rather than through a linker symbol.
 *
 * g_items is the persistent list, in the save-game work area, at the same
 * address in every overlay. g_items_pending is a staging list in the overlay's
 * own work area, so it moves with WORK_BIAS - S2D's sits 0x20000 higher.
 */
#define g_items         ((u_short *)0x801F267C)
#define g_items_pending ((u_short *)(0x800EAE4C + WORK_BIAS))

/* A third list, in the battle only: the items worth offering, staged out of
   g_items. BtlBuildUsableItems walks the whole inventory and copies across
   every entry whose ItemDef carries USABLE_MARK, and BtlCommitUsableItems
   writes the counts back afterwards - so what the battle's menu edits is this
   list and not the party's.

   Eighty entries, against the inventory's 0x17F. Nothing bounds the build, so
   an inventory with more than eighty usable items in it would run off the end;
   in practice far fewer carry the mark.

   Reached through a linker symbol rather than by hardcoded address, unlike the
   two lists above - the instruction pair the original emits says which. */
extern u_short g_btl_usable_items[];

#define ITEM_SLOTS        0x17F  /* entries in a list                        */
#define ITEM_SEARCH       0x17E  /* how far BtlItemSlot walks - one short of
                                    the array, as the original has it        */
#define USABLE_ITEM_SLOTS 0x50   /* entries in the staged list               */

/* An entry packs the id into the low nine bits and the count into the seven
   above them. */
#define ITEM_ID         0x1FF
#define ITEM_SHIFT      9
#define ITEM_COUNT_MASK 0x7F

/* A count is seven bits but stops here. */
#define ITEM_MAX   99

/* What a search leaves in the low half of its answer, with the slot in the
   high half; a slot is one step of SLOT_STEP along. */
#define SLOT_FULL  0
#define SLOT_EMPTY 1
#define SLOT_SOME  2
#define SLOT_STEP  0x10000

extern int          BtlItemAdd(int id);
extern int          BtlItemRemove(int id);

/* The battle's own pair on the same list, which walk it themselves rather than
   asking BtlItemSlot where the entry is. Giving refuses a stack already at
   ITEM_MAX instead of opening a second one; taking answers whether it found
   anything to take. */
extern void         BtlGiveItem(int id);
extern int          BtlTakeItem(int id);
/* BtlItemSlot answers a packed word. Some callers were built against a
   declaration that narrowed it for them; they cast at the call instead, which
   is the same instruction and keeps one prototype here. */
extern unsigned int BtlItemSlot(u_short id);
/* The mark in ItemDef.unk06 that puts an entry on the staged list. */
#define USABLE_MARK 0x80

/* The same three again, over the staged list. Nothing calls them - the menu
   that would have edits the list directly - but they are what the list is
   shaped for. */
extern int          BtlUsableItemAdd(int id);
extern int          BtlUsableItemRemove(int id);
extern unsigned int BtlUsableItemSlot(u_char id);

#endif
