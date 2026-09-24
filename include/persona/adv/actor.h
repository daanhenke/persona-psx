#ifndef PERSONA_ADV_ACTOR_H
#define PERSONA_ADV_ACTOR_H

/* Persona 1 (JP) - the actors a scene puts in a room.
 *
 * Eight 0x2C-byte records, expanded from the pack's actor definitions. Only
 * the fields a decompiled routine has pinned down are named; the rest is here
 * for the offsets.
 */
#include <decomp/types.h>

typedef struct {
    /* 0x00 */ int     script;          /* what the actor's sprite plays;
                                           -1 for nothing                   */
    /* 0x04 */ u_int   flags;           /* ACTOR_FLIP_OK, ACTOR_SEMITRANS    */
    /* 0x08 */ int     unk08;           /* -1 once the actor is built     */
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
    /* 0x1B */ u_char  bright;          /* its shadow is drawn at half as
                                           much again                       */
    /* 0x1C */ u_char  x, y;
    /* 0x1E */ u_char  next_x, next_y;  /* where the step in progress leads */
    /* 0x20 */ u_char  home_x, home_y;  /* where a room actor was placed  */
    /* 0x22 */ u_char  unk22;
    /* 0x23 */ u_char  unk23;
    /* 0x24 */ u_char  unk24;
    /* 0x25 */ u_char  unk25;
    /* 0x26 */ u_char  unk26;          /* gates the second leg of a diagonal
                                          step the way the tile under the
                                          actor gates the first            */
    /* 0x27 */ u_char  pad27[2];
    /* 0x29 */ u_char  shadow;          /* how the second sprite below the
                                           actor is drawn: SHADOW_*          */
    /* 0x2A */ u_char  pad2A[2];
} AdvActor;                             /* 0x2C bytes */

/* Reached by hardcoded address rather than through the linker symbol.
   ActorsSetDepth walks 25 of them, which is more than the eight a room's own
   actor definitions expand to - the array runs on past them. */
#define g_adv_actors ((AdvActor *)0x801F15D8)
#define ACTOR_COUNT  25

/* An empty slot reads 0xFFFF in its id; ActorAtTile answers 0xFF for "nobody
   is standing there". An actor standing behind another draws behind it, which
   is what the larger depth means. */
#define ACTOR_NONE   0xFFFF
#define ACTOR_NA     0xFF
#define DEPTH_BEHIND 0x20

/* An actor that may be drawn mirrored, when its facing asks for it, and one
   drawn semi-transparent. */
#define ACTOR_FLIP_OK   0x100
#define ACTOR_SEMITRANS 0x200

/* What the second sprite does: nothing, a flattened shadow at the actor's
   feet or one lower down, or a plain copy, at either height. The two brighter
   kinds are the actor's reflection rather than its shadow. */
#define SHADOW_NONE     0
#define SHADOW_FLAT     1
#define SHADOW_FLAT_LOW 2
#define SHADOW_COPY     3
#define SHADOW_COPY_LIT 4

#endif
