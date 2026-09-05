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
    /* 0x0E */ u_short world_x;         /* the renderer subtracts the camera
                                           from these to get a screen position */
    /* 0x10 */ u_short world_y;
    /* 0x12 */ short   z;               /* base sort depth                  */
    /* 0x14 */ short   depth;           /* added to z: 0, or 0x20 for an
                                           actor standing behind another    */
    /* 0x16 */ u_char  pad16;
    /* 0x17 */ u_char  dir;             /* facing: 0 up, 1 down, 2 left,
                                           3 right, indexing g_dir_x/g_dir_y */
    /* 0x18 */ u_char  next_dir;        /* AdvBuildActors sets both from the
                                           same two bits of the definition   */
    /* 0x19 */ u_char  phase;           /* frame of the sixteen-step walk
                                           cycle, indexing g_walk_dx/dy      */
    /* 0x1A */ signed char steps;       /* frames left in the step under way */
    /* 0x1B */ u_char  pad1B;
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
