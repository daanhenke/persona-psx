/* Persona 1 (JP) - asking the scene what is on a tile.
 *   0x8007FA78 SceneApplyEntry
 *   0x80080318 SceneFindStep
 *   0x800803A8 SceneFindTrigger
 *   0x80080440 SceneFindApproach
 *   0x800804F8 SceneTriggerArmed
 *   0x80082F6C SceneTileAt
 *   0x80082F98 SceneTileToward
 *
 * The callers step the player and then ask what is under them; a hit in any of
 * these tables gives them the record's script pointer to run. The walking code
 * asks in a fixed order: an approach record for the tile being entered stops
 * the move outright, and once the step has happened a step record beats a
 * trigger, which is the only one of the three that carries an event flag.
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

/* Where the party lands after an entry, and how the destination is entered:
   0, 2 and 3 come from an entry record, 1 and 4 from the overlay entry. */
extern short  g_adv_enter_mode;
extern short  g_map_id;
extern u_char g_map_pos_x;
extern u_char g_map_pos_y;
extern u_char g_map_unk4;
extern u_char g_map_room;

/* One step per facing: up, down, left, right. Both are added to an
   unsigned coordinate, so the 0xFF entries are the -1s. */
extern const u_char g_dir_x[];
extern const u_char g_dir_y[];

/* Where the party ends up, and how, once it walks onto an entry tile. The
   record is re-indexed for every field rather than held in a pointer. */
void SceneApplyEntry(u_char entry)
{
    switch (g_adv_scene->entries[entry].mode) {
    case 0:
        g_adv_enter_mode = 3;
        break;
    case 1:
        g_adv_enter_mode = 2;
        break;
    case 2:
        g_adv_enter_mode = 0;
        break;
    }
    g_map_id = g_adv_scene->entries[entry].map_id;
    g_map_pos_x = g_adv_scene->entries[entry].map_x;
    g_map_pos_y = g_adv_scene->entries[entry].map_y;
    g_map_unk4 = g_adv_scene->entries[entry].unk4;
    g_map_room = g_adv_scene->entries[entry].room;
}

/* The unconditional script for the tile the player has just stepped onto. */
u_char SceneFindStep(u_char x, u_char y)
{
    AdvStep *s;
    u_char   i;

    for (i = 0; i < *g_adv_scene->step_count; i++) {
        s = &g_adv_scene->steps[i];
        if (x == s->x && y == s->y) {
            return i;
        }
    }
    return TRIGGER_NONE;
}

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

/* The record guarding the tile the player is walking into. `dirs` is the bit
   for the facing the step is being made in, so a doorway can be one-way, and
   `kind` has to match the walking actor's own. */
u_char SceneFindApproach(u_char x, u_char y, u_char dirs, u_char kind)
{
    AdvApproach *p;
    u_char       i;

    for (i = 0; i < *g_adv_scene->approach_count; i++) {
        p = &g_adv_scene->approaches[i];
        if (x == p->x && y == p->y && (dirs & p->dirs) && p->kind == kind) {
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
