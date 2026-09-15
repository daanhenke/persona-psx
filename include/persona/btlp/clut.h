#ifndef PERSONA_BTLP_CLUT_H
#define PERSONA_BTLP_CLUT_H

/* Persona 1 (JP) - tinting a fighter's palette.
 *
 * g_btl_tint_pick_r, _g and _b are one table of colour triples reached a
 * channel at a time. The first triple is the pick tint every aim uses; a
 * record carrying attribute 0x200000 is coloured from the triple its child
 * count picks.
 *
 * The palettes are halfword colours. A unit that moves a whole block at a time
 * works in bytes, and casts the pointer to say so.
 */
#include <decomp/types.h>

/* A fighter's palette: this many colours, and this many bytes. */
#define BTL_CLUT_ENTRIES 0x100
#define BTL_CLUT_BYTES   0x200

/* One palette whole, as bytes, for a copy made by assignment. */
typedef struct {
    u_char b[BTL_CLUT_BYTES];
} BtlClutBlock;

/* Each fighter's palette as it is drawn, as it is walking toward, and as it
   was loaded, one block of BTL_CLUT_ENTRIES colours to a slot - the party's
   from g_btl_actor_clut and the enemies' from g_btl_enemy_clut. */
extern u_short *g_btl_actor_clut;
extern u_short *g_btl_actor_clut_to;
extern u_short *g_btl_actor_clut_base;
extern u_short *g_btl_enemy_clut;
extern u_short *g_btl_enemy_clut_to;
extern u_short *g_btl_enemy_clut_base;

/* Where each slot's palette sits in the TIM it was uploaded from. */
extern u_long *g_btl_slot_clut[];

/* Written data, not read-only: declared const, the scheduler moves their loads
   ahead of stores the image keeps them behind. */
extern u_char g_btl_tint_pick_r[];
extern u_char g_btl_tint_pick_g[];
extern u_char g_btl_tint_pick_b[];

/* Tints fighter slot `actor`'s palette toward one colour. tintclut.c. */
extern void BtlTintActorClut(int actor, int r, int g, int b);

#endif
