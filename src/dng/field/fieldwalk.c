/* Persona 1 (JP) - walking and turning on the field.  DNG only.
 *   0x80068C28 FieldWalk
 *   0x80068EE4 FieldTurn
 *   0x80068FB0 FieldSetHeading
 *
 * A step is nine frames of FieldStepView with the field redrawn after each,
 * the tile under the party changing half way; a turn is nine frames of the
 * heading swinging round and a snap to the exact quarter at the end.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/dng/field.h>

/* Tries to take one step towards walk_dir. Returns `ret` if the party moved,
   or the step was held, and 0 if something stopped it - in which case the
   tile is put back and the field redrawn standing. */
int FieldWalk(int ret)
{
    int i;

    g_walk_from_x = g_dng->pos[POS_X];
    g_walk_from_y = g_dng->pos[POS_Y];
    if (FieldTileOneWay() == 0) {
        g_walk_undo = g_dng->pos[g_dir_axis[g_dng->walk_dir]];
        g_dng->pos[g_dir_axis[g_dng->walk_dir]] += g_dir_tile_step[g_dng->walk_dir];
        if (FieldTileSolid() == 0 && FieldUseTile() == 0) {
            if (g_field_mode != 0) {
                g_dng->pos[g_dir_axis[g_dng->walk_dir]] = g_walk_undo;
                return ret;
            }
            if (!(g_bgm_flags & BGM_RESUMED)) {
                g_bgm_flags |= BGM_RESUMED;
                if (g_bgm_off == 0) {
                    SsSeqReplay(g_bgm_seq);
                    SsSeqSetRitardando(g_bgm_seq, 0x70, 1);
                }
            }
            g_scene->from_x = g_walk_from_x;
            g_scene->from_y = g_walk_from_y;
            if (g_tile_flags != g_tile_defs[g_floor_grid[g_dng->pos[POS_Y]][g_dng->pos[POS_X]]].flags) {
                FieldOpenDoor();
            }
            FieldStepBegin();
            for (i = 0; i < 5; i++) {
                FieldStepView();
                FieldFrame();
            }
            FieldZoneTunes(0);
            for (i = 0; i < 4; i++) {
                FieldStepView();
                FieldFrame();
            }
            FieldStepEnd();
            FieldStepTick();
            return ret;
        }
        g_dng->pos[g_dir_axis[g_dng->walk_dir]] = g_walk_undo;
    }
    FieldPauseBgm();
    FieldBumpWall();
    return 0;
}

/* Turns the party a quarter, `turn` being 1 or -1. */
void FieldTurn(int turn)
{
    int  i;
    long start;

    FieldPauseBgm();
    start = g_dng->angle;
    for (i = 0; i < 9; i++) {
        g_dng->angle += turn * TURN_SPEED;
        FieldSetHeading(turn);
        FieldFrame();
    }
    g_dng->angle = (start + turn * QUARTER_TURN) & 0xFFF;
    FieldSetHeading(turn);
    g_dng->facing = (g_dng->facing - turn) & 3;
}

/* Points the saved view along the party's angle from the tile it stands on:
   the eye a little behind the tile's centre, the target well ahead. The
   backdrop turns the other way by `turn`'s share. */
void FieldSetHeading(int turn)
{
    g_scene->sin = rsin(g_dng->angle);
    g_scene->cos = rcos(g_dng->angle);
    g_dng->view.vpx = g_dng->pos[POS_X] * STEP_LEN - ((g_scene->cos * 150) >> 12);
    g_dng->view.vpz = -g_dng->pos[POS_Y] * STEP_LEN - ((g_scene->sin * 150) >> 12);
    g_dng->view.vrx = g_dng->view.vpx + ((g_scene->cos * 5000) >> 12);
    g_dng->view.vrz = g_dng->view.vpz + ((g_scene->sin * 5000) >> 12);
    g_scene->layers[LAYER_SKY].bg.scrollx -= turn << 5;
}
