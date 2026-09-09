/* Persona 1 (JP) - taking a way out, and naming it on screen.
 *   0x8007FA78 SceneApplyEntry
 *   0x8007FB90 SceneDrawEntryLabel
 *
 * The head of the scene-lookup unit. The two diagonal step routines sit
 * between this and the lookups themselves - they are diagstep.c - and the
 * lookups are in scenelookup.c, with the room grid in scenetile.c.
 *
 * SceneFindEntry is deliberately not declared here. It is defined further
 * along in the original, so the call below has no prototype in scope, and
 * giving it one changes the code that comes out.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/adv/actor.h>
#include <persona/adv/scene.h>
#include <persona/common/slot.h>

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

/* Where the party lands after an entry, and how the destination is entered:
   0, 2 and 3 come from an entry record, 1 and 4 from the overlay entry. */
extern short  g_adv_enter_mode;
extern short  g_map_id;
extern u_char g_map_pos_x;
extern u_char g_map_pos_y;
extern u_char g_map_unk4;
extern u_char g_map_room;

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
