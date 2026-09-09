/* Persona 1 (JP) - the battle's effect slots.
 *
 * Four slots, each either a pointer to an effect record or -1 for free. The
 * draw pass walks all four, dispatching on the low nibble of the kind byte, and
 * a handler that reports itself finished has its slot cleared and its running
 * bit taken away.
 *
 * Only the fields the code named so far touches are spelt out; the record is
 * bigger than this.
 */
#ifndef PERSONA_BTLP_EFFECT_H
#define PERSONA_BTLP_EFFECT_H

#include <decomp/types.h>

/* A piece of an effect: the chain hanging off BtlEffect.next, and what the
   draw pass walks to put an effect's text on screen. The low nibble of `kind`
   picks the drawer, and the position is in eight-pixel steps from the effect's
   own origin. */
typedef struct BtlEffectRow {
    /* 0x0 */ struct BtlEffectRow *next;
    /* 0x4 */ u_char kind;
    /* 0x5 */ u_char row;       /* which row of the effect this is;
                                   BtlEffectOpen starts on row zero */
    /* 0x6 */ u_char x;
    /* 0x7 */ u_char y;
    /* 0x8 */ const u_char *text;
} BtlEffectRow;                 /* 0xC bytes */

typedef struct BtlEffect {
    /* 0x00 */ BtlEffectRow *next;
                                /* the rows this effect is made of, -1 when it
                                   has none. BtlEffectOpen walks the chain for
                                   row zero.                                */
    /* 0x04 */ short   mark;    /* the row byte and the kind beside it, written
                                   as one halfword: -0x100 leaves kind zero and
                                   the row 0xFF, so the record is no longer the
                                   first row of anything                    */
    /* 0x06 */ u_short grid;    /* columns in the low byte, rows in the
                                   high one; the cursor wraps inside it */
    /* 0x08 */ u_short flags;   /* 0x8000 drawn, 0x20 running, 11..12 shift */
    /* 0x0A */ u_char  kind;    /* the low nibble picks the handler         */
    /* 0x0B */ u_char  sel;     /* which cell of the grid the cursor is
                                   on; the row whose number matches is
                                   what the effect steps to           */
    /* 0x0C */ u_char  pad0C[0xC];
    /* 0x18 */ u_short dx;      /* what the shift is a multiple of          */
    /* 0x1A */ short   dy;
    /* 0x1C */ short   curx;    /* where the cursor sits relative to the
                                   effect's own origin                */
    /* 0x1E */ short   cury;
    /* 0x20 */ long    unk20;   /* the nine below are what BtlEffectOpen    */
    /* 0x24 */ long    unk24;   /* puts a fresh record back to; only the    */
    /* 0x28 */ long    unk28;   /* values it writes are known              */
    /* 0x2C */ u_char  pad2C[4];
    /* 0x30 */ short   unk30;
    /* 0x32 */ short   unk32;
    /* 0x34 */ short   unk34;
    /* 0x36 */ u_char  pad36[2];
    /* 0x38 */ long    unk38;
    /* 0x3C */ long    unk3C;
    /* 0x40 */ long    scale;   /* 0x1000 is unity, as everywhere else here */
    /* 0x44 */ u_char  pad44[0x4C];
    /* 0x90 */ u_long  mode[3]; /* a DR_MODE naming the effect's texture    */
    /* 0x9C */ u_long  mode_kept[3];
                                /* a copy of it, taken when the record is
                                   opened                                   */
} BtlEffect;

#define BTL_EFFECT_SLOTS 4
#define BTL_EFFECT_FREE  (-1)

/* Taken away when an effect ends. */
#define BTL_EFFECT_RUNNING 0x20

/* Written over the outgoing effect's mark when another takes over; spelt
   negative, which is how it reaches the halfword in one instruction. */
#define BTL_EFFECT_MARK (-0x100)

/* Bits 11 and 12 of the flags choose how far the effect is shifted. */
#define BTL_EFFECT_SHIFT     11
#define BTL_EFFECT_SHIFT_NONE 0
#define BTL_EFFECT_SHIFT_4X   1
#define BTL_EFFECT_SHIFT_8X   2

extern BtlEffect *g_btl_effect[];

/* Which slot the pad is talking to, the one before it, and the one a caller
   put aside. BTL_EFFECT_FREE means none. */
extern int g_btl_effect_cur;
extern int g_btl_effect_prev;
extern int g_btl_effect_held;

extern void BtlEffectRelease(int slot);
extern void BtlEffectSelect(int slot);
extern void BtlEffectRestore(void);
extern void BtlEffectNext(void);

#endif
