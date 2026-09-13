/* Persona 1 (JP) - the debug HUD.  BTLP only.
 *   0x8008DC68 BtlDrawDebugHud  0x8008E158 BtlDebugPause
 *
 * BtlDrawDebugHud runs at the top of every frame. With g_btl_debug_hud raised
 * it shows the HUD's four objects - the access lamp lit only while the CD is
 * busy - and holding square makes every hit deal 9999; lowered, it hides them
 * all again. Either way it fills their cells every frame: the sprite and
 * polygon counts and their peaks, a 0 or a 1 and a colour for whether each of
 * the sixteen sound banks is loaded, how many presses the escape question
 * took, the play time to a tenth of a second, the camera's angles, distances,
 * shift and scale, how much of the artwork arena is used, and a word saying
 * whether the view is at its natural scale. Start with R2 flips D_800CCAAB,
 * which nothing in the overlay reads.
 *
 * BtlDebugPause has no caller. It holds the game from one press of the page
 * key to the next, reading the first pad every field, and matches the presses
 * against the sequence BtlCheatWatch wants from the second pad; finishing it
 * flips g_btl_debug_hud and ends the pause.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/main/cd.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/debug.h>
#include <persona/btlp/input.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>

#define PAD_START 0x800

/* The play time in the main executable: hours, minutes, seconds and the
   frame, reached by its literal address. */
#define g_playtime ((u_char *)0x801F29BC)
#define PLAY_HOUR  0
#define PLAY_MIN   1
#define PLAY_SEC   2
#define PLAY_FRAME 3
#define PLAY_FRAMES_PER_TENTH 6

/* The two digit glyphs, and the palettes a bank's cells are drawn with. */
#define HUD_GLYPH_0  0xE5
#define HUD_GLYPH_1  0xE6
#define HUD_CLUT_ON  0x21
#define HUD_CLUT_OFF 0x23

#define HUD_BANKS       16
#define HUD_COUNT_WIDTH 3
#define HUD_TIME_WIDTH  2
#define HUD_CAM_WIDTH   5
#define HUD_CAM_CELL    10
#define HUD_SCALE_TEXT  5
#define HUD_SCALE_ONE   0x1000

extern int VSync(int mode);

extern u_char D_800CCAAB;

/* The HUD's cells: three-digit counts, the play time, the camera's values ten
   bytes apart, and the word for the scale. */
extern u_char g_btl_hud_sprite_cells[];
extern u_char g_btl_hud_sprite_peak_cells[];
extern u_char g_btl_hud_poly_cells[];
extern u_char g_btl_hud_poly_peak_cells[];
extern u_char g_btl_hud_escape_cells[];
extern u_char g_btl_hud_hour_cells[];
extern u_char g_btl_hud_min_cells[];
extern u_char g_btl_hud_sec_cells[];
extern u_char g_btl_hud_tenth_cells[];
extern u_char g_btl_hud_gfx_cells[];
extern u_char g_btl_debug_cells[];
extern u_char g_btl_hud_scale_text[];

/* One mark per bank, the text records that draw the marks, and the records
   that draw the banks' names. */
extern u_char     g_btl_hud_vab_marks[HUD_BANKS];
extern BtlGfxText g_btl_hud_vab_mark_cells[HUD_BANKS];
extern BtlGfxText g_btl_hud_vab_name_cells[HUD_BANKS];

/* What the word says at the natural scale and at any other. */
extern const u_char g_btl_hud_scale_unit[];
extern const u_char g_btl_hud_scale_other[];

void BtlDrawDebugHud(void)
{
    u_char *time;
    int     i;

    time = g_playtime;
    if (g_btl_debug_hud != 0 && (g_btl_pad1 & g_btl_key_square)) {
        g_btl_debug_max_damage = 1;
    } else {
        g_btl_debug_max_damage = 0;
    }
    if (g_btl_debug_hud != 0) {
        if (g_cd_busy == -1) {
            g_btl_hud_lamp_lit->attr |= BTL_OBJ_HIDDEN;
        } else {
            g_btl_hud_lamp_lit->attr &= ~BTL_OBJ_HIDDEN;
        }
        g_btl_hud_lamp->attr &= ~BTL_OBJ_HIDDEN;
        g_btl_hud_board_upper->attr &= ~BTL_OBJ_HIDDEN;
        g_btl_hud_board_lower->attr &= ~BTL_OBJ_HIDDEN;
    } else {
        g_btl_hud_lamp->attr |= BTL_OBJ_HIDDEN;
        g_btl_hud_lamp_lit->attr |= BTL_OBJ_HIDDEN;
        g_btl_hud_board_upper->attr |= BTL_OBJ_HIDDEN;
        g_btl_hud_board_lower->attr |= BTL_OBJ_HIDDEN;
    }
    BtlDrawNumberAlt(g_btl_hud_sprite_cells, g_btl_sprite_count, HUD_COUNT_WIDTH);
    BtlDrawNumberAlt(g_btl_hud_sprite_peak_cells, g_btl_sprite_peak, HUD_COUNT_WIDTH);
    BtlDrawNumberAlt(g_btl_hud_poly_cells, g_btl_poly_count, HUD_COUNT_WIDTH);
    BtlDrawNumberAlt(g_btl_hud_poly_peak_cells, g_btl_poly_peak, HUD_COUNT_WIDTH);
    for (i = 0; i < HUD_BANKS; i++) {
        if (g_btl_vab[i] >= 0) {
            g_btl_hud_vab_marks[i] = HUD_GLYPH_1;
            g_btl_hud_vab_mark_cells[i].clut = HUD_CLUT_ON;
        } else {
            g_btl_hud_vab_marks[i] = HUD_GLYPH_0;
            g_btl_hud_vab_mark_cells[i].clut = HUD_CLUT_OFF;
        }
    }
    BtlDrawNumberAlt(g_btl_hud_escape_cells, g_btl_escape_presses, HUD_COUNT_WIDTH);
    BtlDrawNumberAlt(g_btl_hud_hour_cells, time[PLAY_HOUR], HUD_TIME_WIDTH);
    BtlDrawNumberAlt(g_btl_hud_min_cells, time[PLAY_MIN], HUD_TIME_WIDTH);
    BtlDrawNumberAlt(g_btl_hud_sec_cells, time[PLAY_SEC], HUD_TIME_WIDTH);
    BtlDrawNumberAlt(g_btl_hud_tenth_cells,
                     time[PLAY_FRAME] / PLAY_FRAMES_PER_TENTH, 1);
    if (g_btl_hud_min_cells[0] == 0) {
        g_btl_hud_min_cells[0] = HUD_GLYPH_0;
    }
    if (g_btl_hud_sec_cells[0] == 0) {
        g_btl_hud_sec_cells[0] = HUD_GLYPH_0;
    }
    if ((g_btl_pad1 & PAD_START) && (g_btl_pad1_edge & g_btl_key_r2)) {
        D_800CCAAB ^= 1;
    }
    BtlDrawNumberAlt(g_btl_debug_cells, g_btl_cam_rot.vx, HUD_CAM_WIDTH);
    BtlDrawNumberAlt(g_btl_debug_cells + HUD_CAM_CELL, g_btl_cam_rot.vy, HUD_CAM_WIDTH);
    BtlDrawNumberAlt(g_btl_debug_cells + HUD_CAM_CELL * 2, g_btl_intro_dist, HUD_CAM_WIDTH);
    BtlDrawNumberAlt(g_btl_debug_cells + HUD_CAM_CELL * 3, g_btl_screen_dist, HUD_CAM_WIDTH);
    BtlDrawNumberAlt(g_btl_debug_cells + HUD_CAM_CELL * 4, g_btl_cam_shift.vx, HUD_CAM_WIDTH);
    BtlDrawNumberAlt(g_btl_debug_cells + HUD_CAM_CELL * 5, g_btl_cam_shift.vy, HUD_CAM_WIDTH);
    BtlDrawNumberAlt(g_btl_debug_cells + HUD_CAM_CELL * 6, g_btl_draw_dist, HUD_CAM_WIDTH);
    BtlDrawNumberAlt(g_btl_debug_cells + HUD_CAM_CELL * 7, g_btl_view_scale.vx, HUD_CAM_WIDTH);
    BtlDrawNumberAlt(g_btl_debug_cells + HUD_CAM_CELL * 8, g_btl_view_scale.vy, HUD_CAM_WIDTH);
    BtlDrawNumberAlt(g_btl_hud_gfx_cells, g_btl_gfx_next - g_btl_gfx_base, HUD_CAM_WIDTH);
    for (i = 0; i < HUD_BANKS; i++) {
        if (g_btl_vab[i] >= 0) {
            g_btl_hud_vab_name_cells[i].clut = HUD_CLUT_ON;
        } else {
            g_btl_hud_vab_name_cells[i].clut = HUD_CLUT_OFF;
        }
    }
    if (g_btl_view_scale.vx == HUD_SCALE_ONE && g_btl_view_scale.vy == HUD_SCALE_ONE) {
        memcpy(g_btl_hud_scale_text, g_btl_hud_scale_unit, HUD_SCALE_TEXT);
    } else {
        memcpy(g_btl_hud_scale_text, g_btl_hud_scale_other, HUD_SCALE_TEXT);
    }
}

void BtlDebugPause(void)
{
    if (g_btl_pad1_edge & g_btl_key_page) {
        do {
            BtlPadRead();
            if (g_btl_cheat_pad[g_btl_cheat_step] == 0) {
                g_btl_debug_hud ^= 1;
                break;
            }
            if (g_btl_pad1_edge != 0) {
                if (g_btl_cheat_pad[g_btl_cheat_step] & g_btl_pad1_edge) {
                    g_btl_cheat_step++;
                } else {
                    g_btl_cheat_step = 0;
                }
            }
            VSync(0);
        } while ((g_btl_pad1_edge & g_btl_key_page) == 0);
        g_btl_cheat_step = 0;
    }
}
