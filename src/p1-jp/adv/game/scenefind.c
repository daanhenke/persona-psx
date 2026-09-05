/* Persona 1 (JP) - asking the scene what is on a tile.
 *   0x800803A8 SceneFindTrigger
 *   0x80082F6C SceneTileAt
 *
 * The callers step the player and then ask what is under them; a trigger hit
 * gives them the record's script pointer to run.
 */
#include <types.h>
#include <persona/adv/scene.h>

#define TRIGGER_NONE 0xFF
#define ROOM_STRIDE  32

u_char SceneFindTrigger(u_char x, u_char y)
{
    AdvTrigger *t;
    u_char      i;

    for (i = 0; i < *g_adv_scene->trigger_count; i++) {
        t = &g_adv_scene->triggers[i];
        if (x == t->x && y == t->y) {
            return i;
        }
    }
    return TRIGGER_NONE;
}

/* The room grid itself, which is what the walking code tests before it moves. */
u_char SceneTileAt(u_char x, u_char y)
{
    int i;

    i = y * ROOM_STRIDE + x;
    return g_adv_scene->tiles[i];
}
