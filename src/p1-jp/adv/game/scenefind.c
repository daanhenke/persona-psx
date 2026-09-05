/* Persona 1 (JP) - asking the scene what is on a tile.
 *   0x800803A8 SceneFindTrigger
 *   0x80082F6C SceneTileAt
 *   0x80082F98 SceneTileToward
 *   0x800804F8 SceneTriggerArmed
 *
 * The callers step the player and then ask what is under them; a trigger hit
 * gives them the record's script pointer to run.
 */
#include <types.h>
#include <persona/adv/scene.h>

#define TRIGGER_NONE 0xFF
#define ROOM_STRIDE  32

/* Which way round a trigger's event flag arms it. */
#define TRIGGER_WHEN_SET   1
#define TRIGGER_WHEN_CLEAR 2

/* Declared u_char here, so the caller narrows what the flag banks
   return as an int. */
extern u_char EventFlagGet(short id);

/* One step per facing: up, down, left, right. Both are added to an
   unsigned coordinate, so the 0xFF entries are the -1s. */
extern const u_char g_dir_x[];
extern const u_char g_dir_y[];

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

/* What is one step along a facing - the test the walking code makes before it
   commits to a move. */
u_char SceneTileToward(u_char x, u_char y, u_char dir)
{
    int nx;
    int i;

    nx = x + g_dir_x[dir];
    i = (u_char)(y + g_dir_y[dir]) * ROOM_STRIDE + (u_char)nx;
    return g_adv_scene->tiles[i];
}

/* Whether a trigger fires: a record can be armed for the flag being set or for
   it being clear, and one that names neither is inert. */
u_char SceneTriggerArmed(u_char trigger)
{
    if (EventFlagGet(g_adv_scene->triggers[trigger].flag)) {
        return g_adv_scene->triggers[trigger].mode & TRIGGER_WHEN_SET;
    }
    return g_adv_scene->triggers[trigger].mode & TRIGGER_WHEN_CLEAR;
}
