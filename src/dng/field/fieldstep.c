/* Persona 1 (JP) - stepping from one tile of the field to the next.  DNG only.
 *   0x80068694 FieldReload
 *   0x80068750 FieldEnterTile
 *   0x800689DC FieldStepView
 *   0x80068A90 FieldStepEnd
 *   0x80068B60 FieldSaveView
 *   0x80068BA8 FieldStepBegin
 *
 * A step slides the view along the axis the party faces: FieldStepBegin notes
 * where the eye and the target start, FieldStepView moves both a frame's worth
 * and FieldStepEnd puts them exactly one tile on, so the frames never add up
 * short. Each saves the moved axes into the game state, which is what the
 * next load comes back to. FieldEnterTile then does what the new tile asks.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/dng/field.h>

/* The two halves of a door are the third and fourth objects of the tile's
   eight, and open by this far along their axes. */
#define DOOR_OPEN 150

/* The view window the scene holds objects for, in tiles. */
#define WINDOW 11

/* Puts the floor's pictures back in VRAM and restarts the field around the
   party, fading it in. */
void FieldReload(void)
{
    UploadImageRows((void *)(PACK_BASE + g_pack_images[0]), 0, 0x1E1,
                    PACK_INDEX[g_pack_sel]);
    DrawSync(0);
    TimLoad((u_long *)(PACK_BASE + g_pack_tims[2]), 0);
    TimLoad((u_long *)(PACK_BASE + g_pack_tims[3]), 0);
    DrawSync(0);
    func_8006D33C(1);
    g_field_hold = 0;
    FieldEnterTile();
    func_8006A3CC();
}

/* Reads the tile the party stands on. A door is shown already open, and a
   music tile starts the floor's tune - unless the tune is already running
   from here, or the map is one of the two whose music a quest bit silences. */
void FieldEnterTile(void)
{
    DngState *d;
    u_int     flags;
    u_char    cx, cy;
    int       tune;

    d = g_dng;
    flags = g_tile_defs[g_floor_grid[d->pos[POS_Y]][d->pos[POS_X]]].flags;
    g_tile_flags = flags;
    if ((flags & TILE_SPECIAL) && (g_tile_kind = flags & TILE_KIND) == TILE_KIND_DOOR) {
        cy = d->pos[POS_Y] % WINDOW;
        cx = d->pos[POS_X] % WINDOW;
        d->door_obj[0] = (cy * WINDOW + cx) * 8 + 2;
        d->door_obj[1] = (cy * WINDOW + cx) * 8 + 3;
        g_scene->objs[d->door_obj[0]].coord2->coord.t[d->door_axis[0]] += DOOR_OPEN;
        g_scene->objs[d->door_obj[0]].coord2->flg = 0;
        g_scene->objs[d->door_obj[1]].coord2->coord.t[d->door_axis[1]] -= DOOR_OPEN;
        g_scene->objs[d->door_obj[1]].coord2->flg = 0;
    }

    if ((g_dng->map == 1 || g_dng->map == 5) && (g_quest_bits & 0x10)) {
        return;
    }
    /* The tune goes through an int: a short, or the ternary straight into the
       call, loads it unsigned and sign-extends it after the join. */
    if ((g_tile_flags & TILE_MUSIC) && g_music_x >= 0xF0) {
        if (g_map_music[g_dng->map][0] == 2) {
            tune = g_floor_tune_a;
        } else {
            tune = g_floor_tune_b;
        }
        SsPlayBack(tune, 0, 0);
        g_music_x = g_dng->pos[POS_X];
        g_music_y = g_dng->pos[POS_Y];
    }
}

/* One frame of a step. */
void FieldStepView(void)
{
    g_view_eye[g_dir_axis[g_dng->walk_dir]] += g_dir_step[g_dng->walk_dir] * STEP_SPEED;
    g_view_at[g_dir_axis[g_dng->walk_dir]] += g_dir_step[g_dng->walk_dir] * STEP_SPEED;
    FieldSaveView();
}

/* The last frame of a step: exactly a tile on from where it began. */
void FieldStepEnd(void)
{
    g_view_eye[g_dir_axis[g_dng->walk_dir]] = g_dir_step[g_dng->walk_dir] * STEP_LEN + g_step_from_eye;
    g_view_at[g_dir_axis[g_dng->walk_dir]] = g_dir_step[g_dng->walk_dir] * STEP_LEN + g_step_from_at;
    FieldSaveView();
}

/* Copies the two axes a step can move into the game state. */
void FieldSaveView(void)
{
    g_dng->view.vpx = g_view_eye[0];
    g_dng->view.vpz = g_view_eye[2];
    g_dng->view.vrx = g_view_at[0];
    g_dng->view.vrz = g_view_at[2];
}

void FieldStepBegin(void)
{
    g_step_from_eye = g_view_eye[g_dir_axis[g_dng->walk_dir]];
    g_step_from_at = g_view_at[g_dir_axis[g_dng->walk_dir]];
    FieldLoadAhead();
}
