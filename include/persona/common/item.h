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
#include <types.h>

typedef struct {
    /* 0x00 */ int    price;
    /* 0x04 */ u_char pad04[4];
    /* 0x08 */ u_char name[10];   /* packed glyph bytes, ten cells wide */
    /* 0x12 */ u_char pad12[1];
    /* 0x13 */ u_char bonus01;  /* high nibble stat[0], low nibble stat[1] */
    /* 0x14 */ u_char bonus23;  /* high nibble stat[2], low nibble stat[3] */
    /* 0x15 */ u_char bonus4;   /* high nibble stat[4]                     */
    /* 0x16 */ u_char power;
    /* 0x17 */ u_char rate;
    /* 0x18 */ u_char pad18[8];
} ItemDef;                      /* 0x20 bytes */

extern ItemDef g_item_defs[];

#endif
