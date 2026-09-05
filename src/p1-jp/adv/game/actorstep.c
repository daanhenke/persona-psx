/* Persona 1 (JP) - aiming an actor at the next tile.  ADV @ 0x80082FE8.
 *
 * The walking code asks SceneTileToward whether the tile ahead is clear and
 * then calls this to record where the step is going.
 */
#include <types.h>
#include <persona/adv/actor.h>

extern const u_char g_dir_x[];
extern const u_char g_dir_y[];

void ActorStepToward(u_char actor, u_char dir)
{
    AdvActor *a;
    u_char    nx;
    u_char    ny;

    a = &g_adv_actors[actor];
    nx = g_dir_x[dir] + a->x;
    ny = g_dir_y[dir] + a->y;
    a->next_x = nx;
    a->next_y = ny;
}
