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

/* A script that runs because the player is standing on the tile. Unlike a
   trigger it carries no flag, so finding one is enough to run it. */
typedef struct {
    /* 0x00 */ u_char  x, y;
    /* 0x02 */ u_char  pad02[2];
    /* 0x04 */ u_long *script;
} AdvStep;                      /* 8 bytes */

/* A script that runs instead of a step onto its tile, and only from the sides
   `dirs` allows - bit 0 is a step up, 1 down, 2 left, 3 right, matching
   g_dir_x/g_dir_y. `kind` is matched against the walking actor's own. */
typedef struct {
    /* 0x00 */ u_char  x, y;
    /* 0x02 */ u_char  dirs;
    /* 0x03 */ u_char  kind;
    /* 0x04 */ u_long *script;
} AdvApproach;                  /* 8 bytes */

/* Where the player may arrive in this room, keyed by the tile as one u_short
   - the two bytes of an actor's x and y read together. */
typedef struct {
    /* 0x00 */ u_short tile;
    /* 0x02 */ u_char  pad02;
    /* 0x03 */ u_char  map_x;   /* where the party lands on the world map */
    /* 0x04 */ u_char  map_y;
    /* 0x05 */ u_char  mode;    /* how the destination is entered; remapped
                                   into g_adv_enter_mode                  */
    /* 0x06 */ u_short map_id;
    /* 0x08 */ u_char  room;    /* which room of that map                 */
    /* 0x09 */ u_char  unk4;
    /* 0x0A */ u_char  pad0A[4];
} AdvEntry;                     /* 14 bytes */

typedef struct {
    /* 0x00 */ u_char      *step_count;
    /* 0x04 */ AdvStep     *steps;
    /* 0x08 */ u_char      *trigger_count;
    /* 0x0C */ AdvTrigger  *triggers;
    /* 0x10 */ u_char      *approach_count;
    /* 0x14 */ AdvApproach *approaches;
    /* 0x18 */ u_char      *entry_count;
    /* 0x1C */ AdvEntry    *entries;
    /* 0x20 */ u_char      *tiles;    /* the room grid, 32 bytes to a row;
                                        RoomRotatePoint puts the far edge
                                        at 23, so the stride is the power
                                        of two above the room, not its
                                        width */
    /* 0x24 */ u_char       kind;    /* 0..4; picks where the camera starts */
    /* 0x25 */ u_char       pad25;
    /* 0x26 */ u_char       w, h;    /* the room in tiles. The camera only
                                        scrolls while the followed actor is
                                        at least four tiles from either edge,
                                        which is what these bound. */
    /* 0x28 */ u_char       pad28[0xC];
    /* 0x34 */ u_short      map_at;  /* the automap position, already
                                        resolved: the high byte is the base
                                        MapMarkTile adds the room to, and the
                                        low byte is the room. 0xFFFF where the
                                        scene has no automap block at all  */
    /* 0x36 */ u_char       seen_x;  /* the rectangle of that room the scene
                                        reveals on arrival                 */
    /* 0x37 */ u_char       seen_y;
    /* 0x38 */ u_char       seen_w;
    /* 0x39 */ u_char       seen_h;
    /* 0x3A */ u_char       pad3A[2];
} AdvScene;

extern AdvScene *g_adv_scene;

#endif
