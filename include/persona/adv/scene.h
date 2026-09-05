#ifndef PERSONA_ADV_SCENE_H
#define PERSONA_ADV_SCENE_H

/* Persona 1 (JP) - the scene the ADV overlay is running.
 *
 * g_adv_scene points 0x34 into the pack read to 0x80100000, and what the
 * overlay reaches through it are (count, table) pairs pointing back into that
 * pack. Only the tile table's records are worked out; the other two are here
 * for the offsets.
 */
#include <types.h>

/* One tile the player can step on. */
typedef struct {
    /* 0x00 */ u_char  x, y;
    /* 0x02 */ u_char  pad02[2];
    /* 0x04 */ short   flag;    /* event flag the trigger is conditional on */
    /* 0x06 */ u_char  mode;    /* tested a bit at a time */
    /* 0x07 */ u_char  pad07;
    /* 0x08 */ u_long *script;  /* what running the trigger executes */
} AdvTrigger;                   /* 12 bytes */

typedef struct {
    /* 0x00 */ u_char      pad00[8];
    /* 0x08 */ u_char     *trigger_count;
    /* 0x0C */ AdvTrigger *triggers;
    /* 0x10 */ u_char     *count10;   /* 8-byte records keyed by three bytes */
    /* 0x14 */ u_char     *table14;
    /* 0x18 */ u_char     *count18;   /* 14-byte records keyed by a u16     */
    /* 0x1C */ u_char     *table1C;
    /* 0x20 */ u_char     *tiles;     /* the room grid, 32 bytes to a row;
                                        RoomRotatePoint puts the far edge
                                        at 23, so the stride is the power
                                        of two above the room, not its
                                        width */
} AdvScene;

extern AdvScene *g_adv_scene;

#endif
