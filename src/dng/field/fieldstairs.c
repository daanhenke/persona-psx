/* Persona 1 (JP) - stairs, bumps and what a tile allows.  DNG only.
 *   0x80069148 FieldStairs
 *   0x80069500 FieldHop
 *   0x800695D8 FieldStepTick
 *   0x8006966C FieldTileOneWay
 *   0x80069708 FieldTileSolid
 *   0x800697B0 FieldBumpWall
 *   0x80069A14 FieldLoadAhead
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <persona/common/automap.h>
#include <persona/dng/field.h>

/* Sets the automap bit of tile (x, y) of room n, counting rooms from `seen`. */
static inline void MarkSeen(u_char *seen, u_short area, u_short room, u_char x, u_char y)
{
    seen[y * MAP_ROW_BYTES + (g_map_base[area] + room) * MAP_BYTES + (x >> 3)] |= 0x80 >> (x & 7);
}

/* Keep column, room and bit calculations separate for conditional marks
   on the adjacent floor; their lifetimes preserve the original loads. */
static inline void MarkAdjacent(u_char *seen, u_short area, u_short room, u_char x, u_char y)
{
    int tile;
    int col, bit;

    col = x >> 3;
    tile = g_map_base[area] + room;
    bit = x & 7;
    seen[y * MAP_ROW_BYTES + tile * MAP_BYTES + col] |= 0x80 >> bit;
}

/* The automap bit of the party's tile, in the room `seen` starts at. */
#define MARK_SEEN(seen) MarkSeen(seen, g_dng->area, g_dng->room, g_dng->pos[POS_X], g_dng->pos[POS_Y])
#define MARK_ADJACENT(seen) MarkAdjacent(seen, g_dng->area, g_dng->room, g_dng->pos[POS_X], g_dng->pos[POS_Y])

/* The rooms above and below this one in the automap. */
#define SEEN_BELOW (g_map_seen - MAP_BYTES)
#define SEEN_ABOVE (g_map_seen + MAP_BYTES)

/* Takes a flight of stairs, `dir` 1 going up and -1 going down: two tiles
   along the walk direction with a hop on each, marking both on the automap
   of this floor and, when the stairs lead off it, of the next. */
void FieldStairs(int dir)
{
    int i;

    g_dng->pos[g_dir_axis[g_dng->walk_dir]] += g_dir_tile_step[g_dng->walk_dir];
    MARK_SEEN(g_map_seen);
    if (g_field_mode & FIELD_MODE_DOWN) {
        MARK_ADJACENT(SEEN_BELOW);
    } else {
        MARK_ADJACENT(SEEN_ABOVE);
    }
    if (!(g_bgm_flags & BGM_RESUMED)) {
        g_bgm_flags |= BGM_RESUMED;
        if (g_bgm_off == 0) {
            SsSeqReplay(g_bgm_seq);
            SsSeqSetRitardando(g_bgm_seq, 0x70, 1);
        }
    }
    FieldStepBegin();
    for (i = 0; i < 4; i++) {
        FieldStepView();
        FieldFrame();
    }
    FieldZoneTunes(0);
    FieldHop(dir);
    FieldStepEnd();

    if (dir == 1) {
        if (!(g_field_mode & FIELD_MODE_DOWN)) {
            g_stair_floor = dir;
        } else {
            g_stair_floor = 0;
        }
    } else if (!(g_field_mode & FIELD_MODE_DOWN)) {
        g_stair_floor = 0;
    } else {
        g_stair_floor = 2;
    }

    g_view_eye[0] = g_dng->view.vpx;
    g_view_eye[1] = g_dng->view.vpy;
    g_view_eye[2] = g_dng->view.vpz;
    g_view_at[0] = g_dng->view.vrx;
    g_view_at[1] = g_dng->view.vry;
    g_view_at[2] = g_dng->view.vrz;
    g_dng->pos[g_dir_axis[g_dng->walk_dir]] += g_dir_tile_step[g_dng->walk_dir];
    if (g_stair_floor != 0) {
        if (g_field_mode & FIELD_MODE_DOWN) {
            MARK_ADJACENT(SEEN_BELOW);
        } else {
            MARK_ADJACENT(SEEN_ABOVE);
        }
    }
    FieldStepBegin();
    FieldHop(dir);
    for (i = 0; i < 4; i++) {
        FieldStepView();
        FieldFrame();
    }
    FieldStepEnd();
    g_field_mode = 0;
}

/* Four frames of the step bouncing the view up and down by `dir`, then
   leaving it a stair's height up or down; the tune runs faster meanwhile. */
void FieldHop(int dir)
{
    int i;
    int bob;

    SsSetTempo(g_bgm_seq, 0, 0xF0);
    bob = -10;
    for (i = 0; i < 4; i++) {
        bob ^= -4;
        FieldStepView();
        g_dng->view.vpy -= dir * (bob + 15);
        g_dng->view.vry = g_dng->view.vpy;
        FieldFrame();
    }
    g_dng->view.vpy = g_view_eye[1] - dir * 75;
    g_dng->view.vry = g_dng->view.vpy;
    SsSetTempo(g_bgm_seq, 0, 0x70);
}

/* After every step: while the tick flags are all set, advance the step clock
   and count down an effect that lasts so many steps. */
int FieldStepTick(void)
{
    u_char *p;

    if ((g_dng->tick_flags & 0xF) == 0xF) {
        MOON_PHASE = (MOON_PHASE + 1) & 0xF;
        FieldSetMoonIcon();
        if (EFFECT_STEPS != 0) {
            if (--EFFECT_STEPS == 0) {
                g_effect_over = 1;
            }
        }
    }
}

/* Whether the tile the party stands on refuses to be left towards
   walk_dir. */
int FieldTileOneWay(void)
{
    int flags;

    if (g_noclip) {
        return 0;
    } else {
        flags = g_tile_defs[g_floor_grid[g_dng->pos[POS_Y]][g_dng->pos[POS_X]]].flags;
        if ((flags & TILE_KIND) == TILE_KIND_ONEWAY) {
            if ((flags >> (g_dng->walk_dir + 5)) & 1) return 1;
        }
        return 0;
    }
}

/* Whether the party's tile is one it cannot stand on. */
int FieldTileSolid(void)
{
    u_char x, y, t;

    x = g_dng->pos[POS_X];
    if (x >= FLOOR_W) {
        return 1;
    }
    y = g_dng->pos[POS_Y];
    if (y >= FLOOR_W) {
        return 1;
    }
    if (g_noclip) {
        return 0;
    }
    t = g_floor_grid[y][x];
    if (t == 0 || (g_tile_defs[t].flags & TILE_SOLID)) {
        return 1;
    }
    return 0;
}

/* Walks the view three frames into the wall and two back, with a thud, then
   puts it back where it was. */
void FieldBumpWall(void)
{
    int  i;
    long eye, at;

    eye = g_view_eye[g_dir_axis[g_dng->walk_dir]];
    at = g_view_at[g_dir_axis[g_dng->walk_dir]];
    SsPlayBack(g_bump_seq, 0, 1);
    for (i = 0; i < 3; i++) {
        g_view_eye[g_dir_axis[g_dng->walk_dir]] += g_dir_step[g_dng->walk_dir] * BUMP_SPEED;
        g_view_at[g_dir_axis[g_dng->walk_dir]] += g_dir_step[g_dng->walk_dir] * BUMP_SPEED;
        FieldSaveView();
        FieldFrame();
    }
    for (i = 0; i < 2; i++) {
        g_view_eye[g_dir_axis[g_dng->walk_dir]] -= g_dir_step[g_dng->walk_dir] * BUMP_SPEED;
        g_view_at[g_dir_axis[g_dng->walk_dir]] -= g_dir_step[g_dng->walk_dir] * BUMP_SPEED;
        FieldSaveView();
        FieldFrame();
    }
    g_view_eye[g_dir_axis[g_dng->walk_dir]] = eye;
    g_view_at[g_dir_axis[g_dng->walk_dir]] = at;
    FieldSaveView();
}

/* Brings in the scene objects ahead of the party's new tile: a column when
   it faces along x, a row when along z. */
void FieldLoadAhead(void)
{
    FieldSetCell(g_dng->pos[POS_X], g_dng->pos[POS_Y]);
    if (g_dng->walk_dir & 1) {
        FieldLoadColumn();
    } else {
        FieldLoadRow();
    }
}
