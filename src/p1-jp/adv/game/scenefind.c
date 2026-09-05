/* Persona 1 (JP) - finding the trigger on a tile.  ADV @ 0x800803A8.
 *
 * The callers step the player and then ask what is under them; a hit gives
 * them the record's script pointer to run.
 */
#include <types.h>
#include <persona/adv/scene.h>

#define TRIGGER_NONE 0xFF

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
