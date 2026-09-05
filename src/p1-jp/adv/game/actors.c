/* Persona 1 (JP) - looking actors up and stepping them.  ADV only.
 *   0x800830C0 ActorAtTile      0x80082FE8 ActorStepToward
 *   0x8008305C ActorFindAt      0x80083150 ActorsSetDepth
 *   0x80083310 ActorSetTile
 *
 * The walking code asks SceneTileToward whether the tile ahead is clear, then
 * ActorAtTile whether anybody is standing on it, and only then records where
 * the step is going.
 */
#include <types.h>
#include <persona/adv/actor.h>

#define ACTOR_NONE   0xFFFF
#define ACTOR_NA     0xFF
#define DEPTH_BEHIND 0x20

/* The room is drawn isometrically: one tile step is 21 pixels of x and 7 of y,
   with a half-pixel of x carried per row. */
#define TILE_X       21
#define TILE_Y       7
#define DEPTH_BASE   0x440
#define ROOM_W       0x20

extern u_short g_room_origin_x;
extern u_short g_room_origin_y;

extern const u_char g_dir_x[];
extern const u_char g_dir_y[];

/* Skips slots whose id reads 0xFFFF, and walks more records than a room's own
   actors fill. */
u_char ActorAtTile(u_char x, u_char y)
{
    u_char i;

    for (i = 0; i < 32; i++) {
        if (x == g_adv_actors[i].x && y == g_adv_actors[i].y
                && g_adv_actors[i].id != ACTOR_NONE) {
            return i;
        }
    }
    return ACTOR_NA;
}

/* The same search without the bound or the id test: it runs off the end of the
   array if nothing matches. */
u_char ActorFindAt(u_char x, u_char y)
{
    u_char i;

    for (i = 0; ; i++) {
        if (x == g_adv_actors[i].x && y == g_adv_actors[i].y) {
            return i;
        }
    }
}

/* The y step is spelled out in two statements on purpose - do not fold it
   back into one. */
void ActorStepToward(u_char actor, u_char dir)
{
    AdvActor *a;
    u_char    nx;
    u_char    ny;

    a = &g_adv_actors[actor];
    nx = g_dir_x[dir] + a->x;
    ny = g_dir_y[dir];
    ny = ny + a->y;
    a->next_x = nx;
    a->next_y = ny;
}

/* Puts an actor on a tile and works out where that lands on screen. A tile
   nearer the camera gets the larger sort depth. */
void ActorSetTile(short x, short y, AdvActor *a)
{
    short   d;
    u_short ox;
    u_short oy;

    ox = g_room_origin_x + x * TILE_X;
    d = DEPTH_BASE - y;
    d = d - (ROOM_W - x) * 32;
    oy = g_room_origin_y;
    a->y = y;
    a->z = d;
    a->x = x;
    a->flags = 0;
    a->world_x = ox + y * TILE_X + y / 2;
    a->world_y = oy + y * TILE_Y - x * TILE_Y;
}

/* Anything standing behind the named actor draws behind it. */
void ActorsSetDepth(u_short actor)
{
    AdvActor *a;
    int       y;

    y = g_adv_actors[actor].y;
    /* The end test is a signed comparison in the original, hence the casts. */
    for (a = g_adv_actors; (long)a < (long)&g_adv_actors[ACTOR_COUNT]; a++) {
        if (a->id == ACTOR_NONE) {
            a->depth = 0;
        } else if (y >= a->y) {
            a->depth = DEPTH_BEHIND;
        } else {
            a->depth = 0;
        }
    }
}
