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
    /* 0x00 */ u_char  pad00[0x1C];
    /* 0x1C */ u_char  x, y;
    /* 0x1E */ u_char  next_x, next_y;  /* where the step in progress leads */
    /* 0x20 */ u_char  pad20[0xC];
} AdvActor;                             /* 0x2C bytes */

/* Reached by hardcoded address rather than through the linker symbol. */
#define g_adv_actors ((AdvActor *)0x801F15D8)

#endif
