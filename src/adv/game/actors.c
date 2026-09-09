/* Persona 1 (JP) - looking actors up and stepping them.  ADV only.
 *   0x80082FE8 ActorStepToward  0x8008305C ActorFindAt
 *   0x800830C0 ActorAtTile      0x80083150 ActorsSetDepth
 *
 * The walking code asks SceneTileToward whether the tile ahead is clear, then
 * ActorAtTile whether anybody is standing on it, and only then records where
 * the step is going.
 *
 * Two routines further along in this unit are not worked out yet, so ADV takes
 * those stretches from asm and the rest of the unit is in actorstile.c and
 * actorswalk.c.
 */
#include <decomp/types.h>
#include <persona/adv/actor.h>
#include <persona/adv/room.h>

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
