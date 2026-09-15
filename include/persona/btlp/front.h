#ifndef PERSONA_BTLP_FRONT_H
#define PERSONA_BTLP_FRONT_H

/* Persona 1 (JP) - the layer in front of the battle.
 *
 * BtlDrawFront runs it once a frame. What it would animate is posted through
 * BtlFrontPost into a fifteen-entry queue, and four slots at 0x80178000 -
 * outside the overlay, reached by number rather than by name - carry the
 * scales BtlFrontSlotsTick opens and closes. Nothing in the shipped overlay
 * posts or ticks, so the step stays at nought.
 */
#include <decomp/types.h>

typedef struct {
    /* 0x00 */ u_char  unk0[0x10];
    /* 0x10 */ int     unk10;
    /* 0x14 */ int     unk14;
    /* 0x18 */ u_char  unk18[6];
    /* 0x1E */ u_short flags;
    /* 0x20 */ u_short unk20;
    /* 0x22 */ u_char  unk22[6];
    /* 0x28 */ int     unk28;
    /* 0x2C */ u_char  unk2C[0x28];
    /* 0x54 */ int     unk54;
    /* 0x58 */ int     unk58;
    /* 0x5C */ u_char  unk5C[0x90];
} BtlFrontSlot;                     /* 0xEC bytes */

/* The image adds the index before the base, which is maspsx's spelling of a
   numeric address rather than a symbol. */
#define BTL_FRONT_SLOTS ((BtlFrontSlot *)0x80178000)
#define BTL_FRONT_SLOT_COUNT 4

/* The queue holds this many ids and a -1 behind the last. */
#define BTL_FRONT_QUEUE 15

/* A slot's mode word: the low byte is 1 opening or 2 closing, and this bit
   says its scale has still to be started. */
#define BTL_FRONT_MODE_START 0x200

extern short   g_btl_front_step;
extern short   g_btl_front_timer;
extern int     g_btl_front_side;
extern int     g_btl_front_flag;
extern int     g_btl_front_queue[];
extern u_short g_btl_front_modes[];
extern int     g_btl_front_slot_phase;

/* A 16 by 8 block of 15-bit colours, uploaded at (0x300, 0x1E8) by the first
   post. */
extern u_short g_btl_front_clut[];

extern int  BtlDrawFront(u_long *ot);
extern void BtlFrontSlotSet(int slot, int a, int b);
extern int  BtlFrontPost(int id);

#endif
