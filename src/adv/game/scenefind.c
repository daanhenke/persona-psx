/* Persona 1 (JP) - walking the leader, taking a way out, and naming it.
 *   0x8007F244 LeaderWalkStep
 *   0x8007FA1C SceneLeaderArrive
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
#define g_map_room (*(u_char *)0x801F5355)
extern u_short g_adv_walk_dir;

#define g_seq_handle (*(short *)0x801F5390)

extern void SsSeqStop(short seq);
extern void ActorSetStandSprite(u_char a);

#define L g_adv_actors[0]

/* The leader's kind while it stands and while it walks. */
#define LEADER_STANDS 1
#define LEADER_WALKS  2

#define SHADOW_SLOT  24
#define TILE_SLOPE_V 7
#define TILE_SLOPE_H 8
#define TRIGGER_NONE 0xFF

/* Frames a stopped walk waits before it tries to slide round a corner. */
#define WALK_HOLD 0x14

extern u_char   g_walk_hold;
extern u_short  g_key_dash;
extern u_char   g_trigger_dirs[];  /* by walk direction: bit 0 refuses a
                                      trigger whose unk2 is clear */
extern int      g_pad_held[];
extern void    *g_actor_defs[];    /* by kind * 4 + facing */
extern void    *g_walk_defs[];     /* by facing; +4 for the shadow's */
extern u_char   g_dir_flip[];
extern Slot    *g_slot_cur;

extern void   SsSeqPlay(short seq, char mode, short times);
extern void   SlotInit(void *def, u_char slot, int attr, short x, short y);
extern void   ActorStepToward(u_char actor, u_char dir);
extern u_char ActorAtTile(u_char x, u_char y);
extern u_char SceneTileAt(u_char x, u_char y);
extern u_char SceneTileToward(u_char x, u_char y, u_char dir);
extern u_char SceneFindTrigger(u_char x, u_char y);
extern u_char SceneTriggerArmed(u_char trigger);
extern u_char SceneTryDiagCW(AdvActor *a);
extern u_char SceneTryDiagCCW(AdvActor *a);
extern void   WalkAdvance(u_short *wy, u_short *wx, u_char dir, int phase,
                          u_char steps);
extern void   WalkSlopeAdvance(u_short *wy, u_short *wx, u_char dir,
                               u_char phase, u_char steps, u_char slope);
extern void   CamFollowStep(void);

/* One frame of the player walking the leader in g_adv_walk_dir. The tile
   ahead decides whether it goes: nobody may be standing there, and the
   tile's kind has its rules (see diagstep.c). A step that goes starts the
   walking sprite if it was not already walking this way and moves a frame;
   one that does not waits WALK_HOLD frames - fewer with the dash key held -
   and then tries the two diagonals, standing the leader still if neither
   is open. */
void LeaderWalkStep(void)
{
    u_char here;
    u_char trigger;
    int    turn;
    u_short d;

retry:
    ActorStepToward(0, g_adv_walk_dir);
    if (ActorAtTile(L.next_x, L.next_y) >= 0x10) {
        here = SceneTileAt(L.x, L.y);
        switch (SceneTileToward(L.x, L.y, g_adv_walk_dir)) {
        case 4:
            if (L.lift == 0) goto blocked;
        case 2:
            trigger = SceneFindTrigger(L.next_x, L.next_y);
            if (trigger == TRIGGER_NONE) goto blocked;
            if (SceneTriggerArmed(trigger)) goto blocked;
            if (g_adv_scene->triggers[trigger].pad02[0] != 0) goto open;
            if (g_trigger_dirs[g_adv_walk_dir] & 1) goto blocked;
            goto open;
        case 5:
            if (L.dir != g_adv_walk_dir || L.kind == LEADER_STANDS) {
                L.kind = LEADER_WALKS;
                L.dir = g_adv_walk_dir;
                SsSeqPlay(g_seq_handle, 1, 0);
                SlotInit(g_actor_defs[L.kind * 4 + g_adv_walk_dir], 0, L.z,
                         L.world_x, L.world_y);
                switch (L.shadow) {
                case SHADOW_FLAT:
                    SlotInit(g_walk_defs[g_adv_walk_dir + 4], SHADOW_SLOT,
                             L.z, L.world_x, L.world_y);
                    break;
                case SHADOW_FLAT_LOW:
                    SlotInit(g_walk_defs[g_adv_walk_dir], SHADOW_SLOT, L.z,
                             L.world_x, L.world_y);
                    break;
                }
                if (g_dir_flip[g_adv_walk_dir]) {
                    g_slot_cur = &g_slots[0];
                    g_slot_cur->attr |= SLOT_ATTR_XSCALE;
                    g_slot_cur = &g_slots[SHADOW_SLOT];
                    g_slot_cur->attr |= SLOT_ATTR_XSCALE;
                }
            }
            goto walk;
        case 8:
            d = g_adv_walk_dir;
            if (d < 2) goto open;
            if (here != TILE_SLOPE_H) goto blocked;
            turn = (u_short)(d - 2) < 2;
            goto gate;
        case 7:
            d = g_adv_walk_dir;
            if ((u_short)(d - 2) < 2) goto open;
            if (here != TILE_SLOPE_V) goto blocked;
            turn = d < 2;
            goto gate;
        case 3:
        case 6:
        case 9:
            turn = L.lift;
        gate:
            if (turn) goto open;
        case 10:
        blocked:
            if (g_walk_hold == 0) {
                {
                    u_char way;

                    way = SceneTryDiagCW(&L);
                    if (way != 0xFF) goto retry;
                    if (SceneTryDiagCCW(&L) != way) goto retry;
                }
            } else {
                g_walk_hold--;
                if ((g_key_dash & g_pad_held[0]) && g_walk_hold != 0) {
                    g_walk_hold--;
                }
            }
            if (L.kind == LEADER_WALKS) {
                SsSeqStop(g_seq_handle);
            }
            L.dir = g_adv_walk_dir;
            ActorSetStandSprite(0);
            L.kind = LEADER_STANDS;
            return;
        default:
            if (here == TILE_SLOPE_H && (u_short)(g_adv_walk_dir - 2) < 2) goto blocked;
            if (here == TILE_SLOPE_V) {
                if (g_adv_walk_dir == 0) goto blocked;
                if (g_adv_walk_dir == 1) goto blocked;
            }
            if (L.lift == 0) goto open;
            if (here == TILE_SLOPE_H && g_adv_walk_dir == 1) goto open;
            if (here == TILE_SLOPE_V && g_adv_walk_dir == 2) goto open;
            if (L.lift != 0xFF) goto blocked;
        }
    open:
        g_walk_hold = WALK_HOLD;
        if (here != TILE_SLOPE_V) {
            if (here != TILE_SLOPE_H) goto start;
            turn = g_adv_walk_dir < 2;
        } else {
            turn = (u_short)(g_adv_walk_dir - 2) < 2;
        }
        if (turn) L.slope = g_adv_walk_dir + 1;
    start:
        if (L.dir != g_adv_walk_dir || L.kind == LEADER_STANDS) {
            L.kind = LEADER_WALKS;
            L.dir = g_adv_walk_dir;
            SsSeqPlay(g_seq_handle, 1, 0);
            SlotInit(g_actor_defs[L.kind * 4 + g_adv_walk_dir], 0, L.z,
                     L.world_x, L.world_y);
            switch (L.shadow) {
            case SHADOW_FLAT:
                SlotInit(g_walk_defs[g_adv_walk_dir + 4], SHADOW_SLOT, L.z,
                         L.world_x, L.world_y);
                break;
            case SHADOW_FLAT_LOW:
                SlotInit(g_walk_defs[g_adv_walk_dir], SHADOW_SLOT, L.z,
                         L.world_x, L.world_y);
                break;
            }
            g_slot_cur = &g_slots[SHADOW_SLOT];
            if (g_dir_flip[g_adv_walk_dir]) {
                g_slot_cur = &g_slots[0];
                g_slot_cur->attr |= SLOT_ATTR_XSCALE;
                g_slot_cur = &g_slots[SHADOW_SLOT];
                g_slot_cur->attr |= SLOT_ATTR_XSCALE;
            }
        }
    walk:
        WalkAdvance(&L.world_y, &L.world_x, L.dir, L.phase, L.steps);
        WalkSlopeAdvance(&L.world_y, &L.world_x, L.dir, L.phase, L.steps,
                         L.slope);
        CamFollowStep();
        L.phase = (L.phase + L.steps) & 0xF;
    } else {
        SsSeqStop(g_seq_handle);
        L.kind = LEADER_STANDS;
        L.dir = g_adv_walk_dir;
        ActorSetStandSprite(0);
    }
}

#undef L

/* The leader comes in through a way in: the footsteps stop and it stands,
   facing the way the player was walking. */
void SceneLeaderArrive(void)
{
    g_adv_actors[0].steps = 1;
    SsSeqStop(g_seq_handle);
    g_adv_actors[0].kind = 1;
    g_adv_actors[0].dir = g_adv_walk_dir;
    ActorSetStandSprite(0);
}

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
