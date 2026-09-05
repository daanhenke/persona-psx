#ifndef PERSONA_ADV_ACTOR_H
#define PERSONA_ADV_ACTOR_H

/* Persona 1 (JP) - the actors a scene puts in a room.
 *
 * Eight 0x2C-byte records, expanded from the pack's actor definitions. Only
 * the fields a decompiled routine has pinned down are named; the rest is here
 * for the offsets.
 */
#include <types.h>

typedef struct {
    /* 0x00 */ u_char  pad00[0xC];
    /* 0x0C */ u_short id;              /* 0xFFFF while the slot is unused  */
    /* 0x0E */ u_char  pad0E[6];
    /* 0x14 */ short   depth;           /* draw-order bias, 0 or 0x20       */
    /* 0x16 */ u_char  pad16[6];
    /* 0x1C */ u_char  x, y;
    /* 0x1E */ u_char  next_x, next_y;  /* where the step in progress leads */
    /* 0x20 */ u_char  pad20[0xC];
} AdvActor;                             /* 0x2C bytes */

/* Reached by hardcoded address rather than through the linker symbol.
   ActorsSetDepth walks 25 of them, which is more than the eight a room's own
   actor definitions expand to - the array runs on past them. */
#define g_adv_actors ((AdvActor *)0x801F15D8)
#define ACTOR_COUNT  25

#endif
