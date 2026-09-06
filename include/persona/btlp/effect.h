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

#include <types.h>

typedef struct {
    /* 0x00 */ u_char  pad00[4];
    /* 0x04 */ short   mark;
    /* 0x06 */ u_char  pad06[2];
    /* 0x08 */ u_short flags;   /* 0x8000 drawn, 0x20 running, 11..12 shift */
    /* 0x0A */ u_char  kind;    /* the low nibble picks the handler         */
    /* 0x0B */ u_char  pad0B[0xD];
    /* 0x18 */ u_short dx;      /* what the shift is a multiple of          */
    /* 0x1A */ short   dy;
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

#endif
