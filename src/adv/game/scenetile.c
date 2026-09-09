/* Persona 1 (JP) - the room grid itself.
 *   0x80082F6C SceneTileAt
 *   0x80082F98 SceneTileToward
 *
 * The tail of the scene-lookup unit, well past the rest of it: what the
 * walking code tests before it commits to a move. The lookups are in
 * scenelookup.c. The grid stride is in persona/adv/scene.h and the per-facing
 * steps in persona/adv/room.h.
 */
#include <decomp/types.h>
#include <persona/adv/room.h>
#include <persona/adv/scene.h>

/* The room grid itself, which is what the walking code tests before it moves. */
u_char SceneTileAt(u_char x, u_char y)
{
    int i;

    i = y * ROOM_STRIDE + x;
    return g_adv_scene->tiles[i];
}

/* What is one step along a facing - the test the walking code makes before it
   commits to a move. The coordinates arrive as ints and are only narrowed
   after the step is added, which is how a step off the left or top edge wraps
   round to the far side of the grid rather than reading behind it. */
u_char SceneTileToward(int x, int y, u_char dir)
{
    int nx;
    u_char cx;
    int i;

    /* The step is loaded before it is added, and the wrapped column gets a
       local of its own. Writing either as one expression costs the match. */
    nx = g_dir_x[dir];
    nx = x + nx;
    cx = nx;
    i = (u_char)(y + g_dir_y[dir]) * ROOM_STRIDE + cx;
    return g_adv_scene->tiles[i];
}
