#ifndef PERSONA_ADV_SCENE_H
#define PERSONA_ADV_SCENE_H

/* Persona 1 (JP) - the scene the ADV overlay is running.
 *
 * g_adv_scene points 0x34 into the pack read to 0x80100000, and what the
 * overlay reaches through it are (count, table) pairs pointing back into that
 * pack. Only the tile table's records are worked out; the other two are here
 * for the offsets.
 */
#include <decomp/types.h>

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
    /* 0x28 */ u_char       pad28[4];
    /* 0x2C */ u_char      *arrive;  /* the script run on arrival, or -1   */
    /* 0x30 */ u_char       pad30[4];
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

/* The head of the pack read to 0x80100000, ahead of the scene block
   g_adv_scene points at. It names the other files the scene needs, most of
   them in two versions: `flag` is a story flag, and once it is set the
   scene takes `alt` instead of `id`. 0xFFFF is "none" for any of them.
   ovl_adv_entry reads the music, AdvPickSceneByFlags the MES entry and
   AdvLoadEventBg the rest. */
typedef struct {
    /* 0x00 */ u_short flag;
    /* 0x02 */ u_short id;
    /* 0x04 */ u_short alt;
} AdvVariant;

typedef struct {
    /* 0x00 */ AdvVariant bgm;         /* the room's music                 */
    /* 0x06 */ u_short    mes_flag[2]; /* pick one of four MES entries:   */
    /* 0x0A */ u_short    mes[4];      /* neither, first, second or both  */
    /* 0x12 */ AdvVariant bg;          /* the EBG entry                   */
    /* 0x18 */ AdvVariant tim_180;     /* images out of that entry, each  */
    /* 0x1E */ AdvVariant tim_280;     /* going to its own VRAM column    */
    /* 0x24 */ AdvVariant tim_300;
    /* 0x2A */ u_short    tim_160;     /* these two are not conditional:  */
    /* 0x2C */ u_short    tim_380;     /* only "none" skips them          */
    /* 0x2E */ u_short    pad2E[3];
} AdvPackHead;                         /* 0x34 bytes, then the scene      */

/* Reached by hardcoded address. */
#define g_adv_pack ((AdvPackHead *)0x80100000)

/* The pack's actor definitions, reached by address. A room's eight actors
   each come in two forms, and an event flag says which one the room shows:
   the second once the flag is set. The eight extras have the same two forms
   in a shorter record; the last two tables only place things. */
typedef struct {
    /* 0x00 */ int     script;
    /* 0x04 */ union {
                   u_int flags;         /* ACTOR_* above the low nibble    */
                   struct {
                       u_char  dir;     /* facing, the low two bits        */
                       u_char  pad;
                       u_short shadow;
                   } b;
               } u;
    /* 0x08 */ u_char  unk22;
    /* 0x09 */ u_char  x, y;
    /* 0x0B */ u_char  bright;
    /* 0x0C */ u_char  lift;
    /* 0x0D */ u_char  pad0D[3];
} AdvActorForm;                          /* 0x10 bytes */

typedef struct {
    /* 0x00 */ u_short flag;             /* 0xFFFF: always the first form  */
    /* 0x02 */ u_short pad02;
    /* 0x04 */ AdvActorForm form[2];
} AdvActorDef;                           /* 0x24 bytes */

typedef struct {
    /* 0x00 */ int     script;
    /* 0x04 */ u_char  unk22;
    /* 0x05 */ u_char  unk23;            /* its high nibble is the flags   */
    /* 0x06 */ u_char  x, y;
    /* 0x08 */ u_char  bright;
    /* 0x09 */ u_char  lift;
    /* 0x0A */ u_char  pad0A[2];
} AdvPropForm;                           /* 0x0C bytes */

typedef struct {
    /* 0x00 */ u_short flag;
    /* 0x02 */ u_short pad02;
    /* 0x04 */ AdvPropForm form[2];
} AdvPropDef;                            /* 0x1C bytes */

typedef struct {
    /* 0x00 */ u_char  x, y;
    /* 0x02 */ u_char  pad02[6];
    /* 0x08 */ u_char  lift;
    /* 0x09 */ u_char  pad09;
} AdvMarkDef;                            /* 10 bytes */

typedef struct {
    /* 0x00 */ u_char  x, y;
    /* 0x02 */ u_char  dir;
    /* 0x03 */ u_char  lift;
} AdvSpotDef;                            /* 4 bytes */

#define ROOM_ACTORS 8
#define g_room_actor_defs ((AdvActorDef (*)[ROOM_ACTORS])0x801001F0)
#define g_prop_defs       ((AdvPropDef *)0x80100AF0)
#define g_mark_defs       ((AdvMarkDef *)0x80100BD0)
#define g_spot_defs       ((AdvSpotDef *)0x80100C20)

/* Which room of the scene is loaded. */
#define g_adv_room (*(u_char *)0x801F5355)

/* Every one of the tile lookups answers 0xFF for "no record here". */
#define TRIGGER_NONE 0xFF

/* The room grid is 32 tiles across. */
#define ROOM_STRIDE  32

/* Which way round a trigger's event flag arms it; a record naming neither is
   inert. */
#define TRIGGER_WHEN_SET   1
#define TRIGGER_WHEN_CLEAR 2

#endif
