/* Persona 1 (JP) - asking the scene what is on a tile.
 *   0x8007FA78 SceneApplyEntry
 *   0x8007FB90 SceneDrawEntryLabel
 *   0x80080318 SceneFindStep
 *   0x800803A8 SceneFindTrigger
 *   0x80080440 SceneFindApproach
 *   0x800804F8 SceneTriggerArmed
 *   0x80080598 SceneFindEntry
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
#include <libgs.h>
#include <persona/adv/actor.h>
#include <persona/adv/scene.h>
#include <persona/common/slot.h>

#define TRIGGER_NONE 0xFF
#define ROOM_STRIDE  32

/* Reached by hardcoded address rather than through the linker symbol. */
#define g_slots ((Slot *)0x800DC10C)

extern Slot *g_slot_cur;

/* The entry label's sprite, and the three 9-byte {length, eight glyphs}
   records it draws from. */
#define LABEL_SLOT  0x28
#define LABEL_Z     0x10
#define LABEL_X     0x10
#define LABEL_Y     0x10
#define LABEL_SIZE  9
#define LABEL_CELLS 8

extern void          g_entry_label_def;
extern GsCELL        g_entry_label_cells[];
extern short         g_entry_label_x;
extern const u_char  g_entry_labels[];

extern void CellsWriteRow(GsCELL *dst, const u_char *src, u_char page,
                          u_short count);

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

/* Names the way out that the player is standing on. Each label is a 9-byte
   record - a length, then eight glyph bytes - and the three read EXIT, FIELD
   and DUNGEON. The sprite's cells are always eight wide, so the length only
   decides where it starts: (8 - len) * 4 centres it in the field.

   The entry's kind is read twice rather than kept, and the length is reached
   backwards off the text pointer, which is what gcc does with the `+ 1` when
   both expressions share it. */
void SceneDrawEntryLabel(void)
{
    u_char entry;
    u_int idx;

    g_slot_cur = &g_slots[LABEL_SLOT];
    entry = SceneFindEntry(g_adv_actors);
    /* The index is copied into a second local before the call. That is what
       narrows it where the original does; indexing with `entry` throughout
       moves the mask past SlotInitTagged and costs the match. */
    idx = entry;
    SlotInitTagged(&g_entry_label_def, LABEL_SLOT, LABEL_Z, LABEL_X, LABEL_Y);
    CellsWriteRow(g_entry_label_cells,
                  &g_entry_labels[g_adv_scene->entries[idx].mode * LABEL_SIZE
                                  + 1],
                  0, LABEL_CELLS);
    g_entry_label_x =
        (LABEL_CELLS
         - g_entry_labels[g_adv_scene->entries[idx].mode * LABEL_SIZE]) * 4
        + 10;
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

/* Whether a trigger fires: a record can be armed for the flag being set or for
   it being clear, and one that names neither is inert. */
u_char SceneTriggerArmed(u_char trigger)
{
    if (EventFlagGet(g_adv_scene->triggers[trigger].flag)) {
        return g_adv_scene->triggers[trigger].mode & TRIGGER_WHEN_SET;
    }
    return g_adv_scene->triggers[trigger].mode & TRIGGER_WHEN_CLEAR;
}
