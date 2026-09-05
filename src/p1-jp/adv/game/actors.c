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

    /* The test needs a basic block of its own for the index to end up in the
       register the original returns; the do/while(0) is what gives it one. */
    for (i = 0; ; i++) {
        do {
            if (x == g_adv_actors[i].x && y == g_adv_actors[i].y) {
                return i;
            }
        } while (0);
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
    a->phase = 0;
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

/* Sixteen frames of walk animation carry an actor exactly one tile: the
   per-frame steps sum to TILE_Y and TILE_X over a full cycle. This adds up
   `steps` frames from `phase` and applies the total to a screen position with
   the signs the facing calls for - the same projection ActorSetTile uses, so
   a walk in progress lands on the tile ActorSetTile would have given it. */
extern const u_char g_walk_dy[];
extern const u_char g_walk_dx[];

void WalkAdvance(u_short *wy, u_short *wx, u_char dir, int phase, u_char steps)
{
    u_short sy;
    u_short sx;
    int     i;

    sy = 0;
    sx = 0;
    while (steps != 0) {
        steps--;
        i = phase & 0xF;
        phase = i + 1;
        sy = g_walk_dy[i] + sy;
        sx = g_walk_dx[i] + sx;
    }
    switch (dir) {
    case 0:
        *wy = *wy - sy;
        *wx = *wx - sx;
        break;
    case 1:
        *wy = sy + *wy;
        *wx = sx + *wx;
        break;
    case 2:
        *wy = sy + *wy;
        *wx = *wx - sx;
        break;
    case 3:
        *wy = *wy - sy;
        *wx = sx + *wx;
        break;
    }
}
