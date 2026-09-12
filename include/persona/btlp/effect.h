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
/* The last word of a row, read one way or the other by the drawer its kind
   picks. */
typedef union BtlEffectRowData {
    long  mask;                 /* a number row: how wide the value at *text
                                   is - 0xFF a byte, 0xFFFF a halfword, -1 a
                                   whole word                              */
    struct {
        short first;            /* a list row: where in the list of lines at
                                   *text to start, and how many to draw    */
        short count;
    } list;
} BtlEffectRowData;

typedef struct BtlEffectRow {
    /* 0x0 */ struct BtlEffectRow *next;
    /* 0x4 */ u_char kind;
    /* 0x5 */ u_char row;       /* which row of the effect this is;
                                   BtlEffectOpen starts on row zero */
    /* 0x6 */ u_char x;
    /* 0x7 */ u_char y;
    /* 0x8 */ const u_char *text;
    /* 0xC */ BtlEffectRowData u;
} BtlEffectRow;                 /* 0x10 bytes */

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
    /* 0x38 */ long    scale_x; /* what the motion handlers wind up and down;
                                   0x1000 is unity, as everywhere else here */
    /* 0x3C */ long    scale_y;
    /* 0x40 */ long    scale;   /* settled at unity when a motion is done   */
    /* 0x44 */ u_char  pad44[0x4C];
    /* 0x90 */ u_long  mode[3]; /* a DR_MODE naming the effect's texture    */
    /* 0x9C */ u_long  mode_kept[3];
                                /* a copy of it, taken when the record is
                                   opened                                   */
} BtlEffect;

/* Where the effects' own colours start in g_btl_clut; the kind byte's high
   nibble picks one of the eight from there. */
#define EFFECT_CLUT 32

/* One per value of the kind byte's low nibble, in the order the table holds
   them. Each moves the box a frame on and answers whether it is still
   running. */
extern int  BtlEffectMotionHold(BtlEffect *e);
extern int  BtlEffectMotionGrow(BtlEffect *e);
extern int  BtlEffectMotionUnroll(BtlEffect *e);
extern int  BtlEffectMotionOpenNow(BtlEffect *e);
extern int  BtlEffectMotionShutNow(BtlEffect *e);
extern int  BtlEffectMotionRoll(BtlEffect *e);
extern int  BtlEffectMotionShrink(BtlEffect *e);
extern int  BtlEffectMotionCollapse(BtlEffect *e);

extern int  BtlDrawGlyphs(const u_char *text, short clut);
extern void BtlEffectDrawRow(const BtlEffectRow *row);
extern void BtlEffectDrawNumber(const BtlEffectRow *row);
extern int  BtlEffectDrawLines(const BtlEffectRow *row);

/* Where the next glyph goes. BtlDrawGlyphs walks x along the line and the
   row drawers step y down. */
extern short g_btl_glyph_x;
extern short g_btl_glyph_y;
extern void BtlFormatHexGlyphs(u_int value, u_char *out, int unused);

/* The sixteen glyph codes a hex digit is written with, '0' to '9' and then
   'A' to 'F'. BtlFormatHexGlyphs takes a copy rather than reading it where it
   lies, which is what the block move at the top of it is. */
extern const u_char g_btl_hex_glyphs[16];

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
