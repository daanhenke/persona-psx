/* Persona 1 (JP) - one frame of the field: the pad, and leaving the floor.
 * DNG only.
 *   0x80067CC8 FieldUpdate
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/main/state.h>
#include <persona/main/cd.h>
#include <persona/dng/field.h>


extern u_char D_8001555C[];
extern void MainMenuOpen(void);
extern void func_80095578(void);
extern void PersonaDataOpen(void);

#define HERE (g_floor_grid[g_dng->pos[POS_Y]][g_dng->pos[POS_X]])

/* A spot event that starts on the tile being stepped off, whatever the
   step. */
#define TILE_SPOT_MASK (TILE_EVENT | TILE_QUIET | TILE_KIND)
#define TILE_SPOT      (TILE_EVENT | TILE_QUIET | 9)

/* Opens a screen over the field: the pad's click, a fade out, the CD left
   to finish, the screen, and the field put back. */
#define OPEN_SCREEN(screen)                                                    \
    FieldPauseBgm();                                                           \
    SsPlayBack(g_seq_handles[4], 0, 1);                                        \
    FieldFadeOut();                                                            \
    while (g_cd_busy != -1) {                                                  \
    }                                                                          \
    screen();                                                                  \
    FieldReload()

/* A frame of the field: saves the view, turns or steps the party as the pad
   says (stepping on the exit ahead leaves for the next scene, or moves the
   party within the map), opens the menus, holds for the pause button, runs
   the stairs a step started, and reads the tile the party ends on. Returns
   -1 once the field is being left, 0 otherwise. */
int FieldUpdate(int noclip)
{
    u_char unused[0x48]; /* the image's frame holds 0x48 bytes nothing touches */
    int  held;
    int  pressed;
    int  i;
    int  r;

    g_view_eye[0] = g_dng->view.vpx;
    g_view_eye[1] = g_dng->view.vpy;
    g_view_eye[2] = g_dng->view.vpz;
    g_view_at[0] = g_dng->view.vrx;
    g_view_at[1] = g_dng->view.vry;
    g_view_at[2] = g_dng->view.vrz;
    held = g_scene->pad_held;
    g_noclip = noclip;
    g_step_kind = 0;
    if (held & PAD_LEFT) {
        FieldTurn(1);
        g_step_kind = 3;
    } else if (held & PAD_RIGHT) {
        FieldTurn(-1);
        g_step_kind = 4;
    } else if (held & BIND(step_right)) {
        g_dng->walk_dir = (g_dng->facing + 1) & 3;
        if ((g_tile_defs[HERE].flags & TILE_SPOT_MASK) == TILE_SPOT && FieldSpotEvent(0)) {
            goto done;
        }
        g_step_kind = FieldWalk(6);
    } else if (held & BIND(step_left)) {
        g_dng->walk_dir = (g_dng->facing - 1) & 3;
        if ((g_tile_defs[HERE].flags & TILE_SPOT_MASK) == TILE_SPOT && FieldSpotEvent(0)) {
            goto done;
        }
        g_step_kind = FieldWalk(6);
    } else if (held & PAD_DOWN) {
        g_dng->walk_dir = (g_dng->facing + 2) & 3;
        if ((g_tile_defs[HERE].flags & TILE_SPOT_MASK) == TILE_SPOT && FieldSpotEvent(0)) {
            goto done;
        }
        g_step_kind = FieldWalk(6);
    } else if (held & PAD_UP) {
        g_dng->walk_dir = g_dng->facing;
        r = FieldStepDoor();
        switch (r) {
        case 1:
            FieldPauseBgm();
            FieldBumpWall();
            return 0;
        case 3:
            return 0;
        case 2:
            FieldTakeExit();
            switch (g_state_next) {
            case GAME_STATE_ADV:
                FieldFadeFxBegin();
                PreloadAdv();
                if (g_dng->exit_bits & 0x40) {
                    SsSetMarkCallback(g_seq_handles[0], 0, D_8001555C);
                }
                FieldFadeFxRun();
                if (g_dng->exit_bits & 0x80) {
                    for (i = 0; i < 3; i++) {
                        if (g_seq_handles[i] != -1) {
                            SsSetNck(g_seq_handles[i]);
                        }
                    }
                    SsVabClose(g_vab_handles[0]);
                }
                for (i = 12; i < FIELD_SEQS; i++) {
                    if (g_seq_handles[i] != -1) {
                        SsSetNck(g_seq_handles[i]);
                    }
                }
                if (g_vab_handles[2] != -1) {
                    SsVabClose(g_vab_handles[2]);
                }
                g_field_lit = 0;
                return -1;
            case GAME_STATE_S2D:
                FieldFadeSeqs();
                FieldIrisFxBegin();
                PreloadS2d();
                FieldIrisFxRun();
                while (g_draw_buf != 0) {
                    FieldIrisFxStep();
                }
                g_field_lit = 0;
                FieldCloseSound();
                return -1;
            case GAME_STATE_DNG:
                if (g_dest_id == g_dng->map) {
                    FieldFadeOut();
                    g_dng->map = g_dest_id;
                    g_dng->floor = g_dest_unk4;
                    g_dng->pos[POS_X] = g_dest_x;
                    g_dng->pos[POS_Y] = g_dest_y;
                    g_dng->facing = g_dest_facings[g_dest_room][0];
                    FieldSetFloor();
                    FieldSetupGfx(0);
                    FieldSyncMusic(0);
                    g_field_lit = 0;
                    for (i = 0; i < 3; i++) {
                        FieldFrame();
                    }
                    FieldFadeIn();
                    return 0;
                }
                FieldFadeSeqs();
                FieldFadeFxBegin();
                PreloadDng();
                FieldFadeFxRun();
                FieldCloseSound();
                g_field_lit = 0;
                return -1;
            case 6:
                FieldFadeSeqs();
                FieldFadeFxBegin();
                FieldFadeFxRun();
                FieldCloseSound();
                g_field_lit = 0;
                return -1;
            }
        }
        g_step_kind = FieldWalk(1);
    } else {
        pressed = g_scene->pad_new;
        if (pressed & BIND(pause)) {
            FieldPauseBgm();
            g_clock_freeze = 1;
            while (g_scene->pad_held & BIND(pause)) {
                FieldFrame();
            }
            g_clock_freeze = 0;
        } else if (pressed & BIND(menu)) {
            OPEN_SCREEN(MainMenuOpen);
        } else if (pressed & BIND(unk14)) {
            OPEN_SCREEN(func_80095578);
        } else if (pressed & BIND(persona)) {
            OPEN_SCREEN(PersonaDataOpen);
        } else {
            FieldPauseBgm();
        }
    }
done:
    if (g_scene->pad_held & BIND(hold)) {
        g_field_hold = 1;
    } else {
        g_field_hold = 0;
    }
    if ((g_field_mode & 0xF) == 1) {
        FieldStairs(1);
    }
    if ((g_field_mode & 0xF) == 2) {
        FieldStairs(-1);
    }
    g_tile_flags = g_tile_defs[HERE].flags;
    g_tile_kind = g_tile_flags & TILE_KIND;
    return 0;
}
