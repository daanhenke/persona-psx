/* Persona 1 (JP) - leaving the floor: exits to other scenes, stairs and
 * doors.  DNG only.
 *   0x8006C6D4 FieldTakeExit
 *   0x8006C9C8 FieldUseTile
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/main/state.h>
#include <persona/common/automap.h>
#include <persona/common/eventflag.h>
#include <persona/dng/field.h>

#define STORY_FLAG(id) ((g_event_flags[(id) / 8] >> ((id) % 8)) & 1)

/* Exit n, read afresh at every use. The search loop indexes it too, and
   loop.c reduces that to the pointer the image steps; n's uses in the loop
   are what give it the register before the reveal's row counter. */
#define EXIT(n) (((FloorExit *)g_floor_spots)[n])

/* Takes the exit on the party's tile (past the first g_exit_skip of them):
   hands the next scene where to put the party, and reveals the part of the
   automap the exit says the party saw on the way. */
void FieldTakeExit(void)
{
    DngState  *d;
    FloorExit *e;
    int        n;
    int        i;
    int        j;
    int        x;

    d = g_dng;
    n = 0;
    for (;;) {
        if (EXIT(n).x == d->pos[POS_X] && EXIT(n).y == d->pos[POS_Y]) {
            if (g_exit_skip == 0) {
                goto found;
            }
            g_exit_skip--;
        }
        n++;
    }
found:
    e = &EXIT(n);
    g_state_next = g_exit_scenes[e->kind & 7];
    if (g_state_next != 6) {
        g_dest_id = e->id[STORY_FLAG(e->flag)];
        g_dest_x = e->to_x;
        g_dest_y = e->to_y;
        g_dest_room = e->room & 7;
        g_dest_unk4 = e->unk4;
    } else {
        g_dng->scene6_id = e->id[STORY_FLAG(e->flag)];
    }
    switch (g_state_next) {
    case GAME_STATE_BTL:
        break;
    case GAME_STATE_S2D:
        break;
    case GAME_STATE_ADV:
        g_dng->exit_bits = EXIT(n).room & 0xC0;
        break;
    }
    for (j = 0; j < EXIT(n).seen_h; j++) {
        for (i = 0; i < EXIT(n).seen_w; i++) {
            x = EXIT(n).seen_x + i;
            *(g_map_seen + (g_map_base[g_dng->area] + g_dng->room) * MAP_BYTES + (EXIT(n).seen_y + j) * MAP_ROW_BYTES + x / 8) |=
                0x80 >> (x % 8);
        }
    }
}

/* Sets the automap bit of tile (x, y) of the party's room. The parameters
   are what loads all four fields before the sum; x unsigned for the srl. */
static inline void MarkSeen(int area, int room, u_int x, int y)
{
    *(g_map_seen + (g_map_base[area] + room) * MAP_BYTES + y * MAP_ROW_BYTES + (x >> 3)) |= 0x80 >> (x & 7);
}

/* The automap bit of the party's tile. */
#define MARK_SEEN() MarkSeen(g_dng->area, g_dng->room, g_dng->pos[POS_X], g_dng->pos[POS_Y])

/* Points the floor's tables at floor `floor` of the pack and rebuilds the
   scene around them. `floor` steps the member, which lets sched lift the
   table's load above the store; the rest reread the floor as DNG_FLOOR. */
#define LOAD_FLOOR(floor)                                                                \
    g_floor_info = (u_char *)(INDEX_BASE + g_index_info_tab[floor]);                     \
    g_floor_grid = (void *)(INDEX_BASE + g_index_grid_tab[DNG_FLOOR]);                   \
    g_floor_objs = (u_char *)(PACK_BASE + g_pack_obj_tab[DNG_FLOOR]);                    \
    g_floor_spots = (u_char *)(PACK_BASE + g_pack_spot_tab[DNG_FLOOR]);                  \
    g_floor_events = (u_char *)(PACK_BASE + g_pack_event_tab[DNG_FLOOR]);                \
    FieldBuildScene(0);                                                                  \
    FieldRebuildMap();                                                                   \
    FieldSyncMusic(0);                                                                   \
    FieldLoadWallCluts()

/* The eye's height on the upper and the lower floor of a flight of stairs,
   and halfway. */
#define EYE_UP   (-300)
#define EYE_DOWN 0
#define EYE_HALF (-150)

/* The objects a door's halves are, of the scene's eight per tile. */
#define WINDOW 11

/* What the special tile the party is stepping onto does: stairs whose end
   is on the next floor load it, a flight within the floor only sets which
   way it goes, and a door starts sliding open. Returns 0. */
int FieldUseTile(void)
{
    DngState *d;
    int       flags;
    u_char    cx, cy;

    if (g_noclip) {
        return 0;
    }
    flags = g_tile_defs[g_floor_grid[g_dng->pos[POS_Y]][g_dng->pos[POS_X]]].flags;
    if (flags & TILE_SPECIAL) {
        switch (flags & TILE_KIND) {
        case 0:
            if (g_dng->view.vpy == EYE_UP) {
                MARK_SEEN();
                LOAD_FLOOR(++g_dng->floor);
                g_dng->view.vpy = EYE_DOWN;
                g_dng->view.vry = EYE_DOWN;
            }
            g_stair_floor = 2;
            break;
        case 1:
            if (g_dng->view.vpy == EYE_DOWN) {
                MARK_SEEN();
                LOAD_FLOOR(--g_dng->floor);
                g_dng->view.vpy = EYE_UP;
                g_dng->view.vry = EYE_UP;
            }
            g_stair_floor = 1;
            break;
        case 2:
            if (g_dng->view.vpy == EYE_HALF) {
                g_field_mode = 1;
            } else {
                g_field_mode = 2;
            }
            break;
        case 3:
            if (g_dng->view.vpy == EYE_HALF) {
                g_field_mode = FIELD_MODE_DOWN | 2;
            } else {
                g_field_mode = FIELD_MODE_DOWN | 1;
            }
            break;
        case TILE_KIND_DOOR:
            d = g_dng;
            cy = d->pos[POS_Y] % WINDOW;
            cx = d->pos[POS_X] % WINDOW;
            g_door_state = 1;
            /* Plain halfword stores: as door_obj[] members they schedule
               ahead of the translation stores, not after them. */
            *(u_short *)((u_char *)d + 0x15CC) = (cy * WINDOW + cx) * 8 + 2;
            *(u_short *)((u_char *)d + 0x15CE) = (cy * WINDOW + cx) * 8 + 3;
            g_door_dx = 15;
            g_door_dy = -15;
            d->door_axis[0] = g_door_axes[d->walk_dir];
            d->door_axis[1] = g_door_axes[d->walk_dir];
            g_door_frames = 10;
            FieldPlayJingle(0x15, 1);
            break;
        }
    }
    return 0;
}
