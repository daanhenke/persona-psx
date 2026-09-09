/* Persona 1 (JP) - asking the scene what is on a tile.
 *   0x80080318 SceneFindStep      0x800803A8 SceneFindTrigger
 *   0x80080440 SceneFindApproach  0x800804F8 SceneTriggerArmed
 *   0x80080598 SceneFindEntry
 *
 * The callers step the player and then ask what is under them; a hit in any of
 * these tables gives them the record's script pointer to run. The walking code
 * asks in a fixed order: an approach record for the tile being entered stops
 * the move outright, and once the step has happened a step record beats a
 * trigger, which is the only one of the three that carries an event flag.
 *
 * The head of this unit is in scenefind.c and the room grid in scenetile.c.
 * TRIGGER_NONE and the two arming modes are in persona/adv/scene.h.
 */
#include <decomp/types.h>
#include <persona/adv/actor.h>
#include <persona/adv/scene.h>

/* Declared u_char here, so the caller narrows what the flag banks
   return as an int. */
extern u_char EventFlagGet(short id);

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

/* Whether a trigger fires: a record can be armed for the flag being set or for
   it being clear, and one that names neither is inert. */
u_char SceneTriggerArmed(u_char trigger)
{
    if (EventFlagGet(g_adv_scene->triggers[trigger].flag)) {
        return g_adv_scene->triggers[trigger].mode & TRIGGER_WHEN_SET;
    }
    return g_adv_scene->triggers[trigger].mode & TRIGGER_WHEN_CLEAR;
}

/* The entry an actor is standing on, or 0xFF. An entry is a place the player
   can leave the room from, keyed by its tile; since an actor's x and y are
   adjacent bytes, the pair reads as one u_short and compares against the
   record's key directly rather than a coordinate at a time. */
u_char SceneFindEntry(const AdvActor *a)
{
    const AdvEntry *entries;
    u_long tile;
    u_char n;
    u_char i;

    /* The counter is zeroed before the count is read and the guard compares
       the two, which is what puts the zero in the register the original uses.
       Writing `n != 0` and dropping the first assignment costs the match. */
    i = 0;
    n = *g_adv_scene->entry_count;
    if (n != i) {
        i = 0;
        tile = *(u_short *)&a->x;
        entries = g_adv_scene->entries;
        do {
            if (tile == entries[i].tile) {
                return i;
            }
            i++;
        } while (i < n);
    }
    return TRIGGER_NONE;
}
