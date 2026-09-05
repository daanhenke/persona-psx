/* Persona 1 (JP) - looking actors up and stepping them.  ADV only.
 *   0x800830C0 ActorAtTile      0x80082FE8 ActorStepToward
 *   0x8008305C ActorFindAt      0x80083150 ActorsSetDepth
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

/* Anything standing behind the named actor draws behind it. */
void ActorsSetDepth(u_short actor)
{
    AdvActor *a;
    int       y;

    y = g_adv_actors[actor].y;
    for (a = g_adv_actors; a < &g_adv_actors[ACTOR_COUNT]; a++) {
        if (a->id == ACTOR_NONE) {
            a->depth = 0;
        } else if (y >= a->y) {
            a->depth = DEPTH_BEHIND;
        } else {
            a->depth = 0;
        }
    }
}
