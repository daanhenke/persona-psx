/* Persona 1 (JP) - the config screen's battle-settings page.
 *
 *   ADV @ 0x8007629C
 *
 * The fifth entry of the settings-items list opens this page. It reads:
 *
 *   heading   BATTLE SETTINGS
 *   row 0     COMMAND CONFIRM       DO / DO NOT        g_options[1]
 *   row 1     MESSAGE SPEED         NORMAL / FAST / OFF g_options[2]
 *   row 2     WINDOW ANIMATION      DO / DO NOT        g_options[0x23]
 *   row 3     AUTO BATTLE SETTINGS  (opens a page of its own)
 *
 * Each value label is a single string holding its alternatives side by side,
 * spaced so that a marker sprite parked on one of them lines up; that is why
 * rows 0 and 2 share one string, both being the same yes/no pair, and why
 * nothing here has to know which value is selected. ConfigPlaceMarkers puts
 * the markers on the right columns afterwards.
 *
 * S2D builds this same source against a work area 0x20000 higher, which is
 * what WORK_BIAS says.
 */
#include <types.h>
#include <persona/common/menuctx.h>

#define g_tilemap0 ((short *)(0x800EE180 + WORK_BIAS))
#define g_tilemap1 ((short *)(0x800EF580 + WORK_BIAS))
#define MAP_W 40

extern const u_char str_cfg_battle[];
extern const u_char str_cfg_command_confirm[];
extern const u_char str_cfg_message_speed[];
extern const u_char str_cfg_window_anim[];
extern const u_char str_cfg_auto_battle[];
extern const u_char str_cfg_do_or_not[];
extern const u_char str_cfg_speeds[];

extern void TileMapFillRect(short *dst, short value, u_short w, u_short h,
                            u_short stride);
extern void TileMapDrawWindow(short *dst, u_short w, u_short h, u_short stride);
extern void TileMapDrawBox(short *dst, u_short w, u_short h, u_short stride);
extern void TileMapWriteRun10(short *dst);
extern void TileMapWriteRow(const u_char *src, short *dst, u_short base,
                            u_short count);
extern void SlotSetPos(u_char slot, int attr, short x, short y);
extern void SlotClear(u_char slot);
extern void ConfigBeginEdit(void);
extern void ConfigPlaceMarkers(void);

/* Where the cursor sits for row n. */
#define CURSOR_SLOT 1
#define CURSOR_Z    0x42
#define CURSOR_X    0x48
#define CURSOR_Y0   0x48
#define ROW_PITCH   0x18

/* Option names start at column 0, their values at column 11. */
#define VALUE_COL 11

void ConfigRedrawBattlePage(void)
{
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x1D, 0xF, MAP_W);
    TileMapDrawBox(&g_tilemap0[MAP_W + 1], 0x1B, 0xD, MAP_W);
    TileMapWriteRun10(&g_tilemap0[2 * MAP_W + 2]);

    TileMapWriteRow(str_cfg_battle, &g_tilemap1[MAP_W], 0, 0x10);
    TileMapWriteRow(str_cfg_command_confirm, &g_tilemap1[3 * MAP_W], 0, 0x10);
    TileMapWriteRow(str_cfg_message_speed, &g_tilemap1[5 * MAP_W], 0, 0x10);
    TileMapWriteRow(str_cfg_window_anim, &g_tilemap1[7 * MAP_W], 0, 0x10);
    TileMapWriteRow(str_cfg_auto_battle, &g_tilemap1[9 * MAP_W], 0, 10);

    TileMapWriteRow(str_cfg_do_or_not, &g_tilemap1[3 * MAP_W + VALUE_COL],
                    0, 8);
    TileMapWriteRow(str_cfg_speeds, &g_tilemap1[5 * MAP_W + VALUE_COL],
                    0, 0xD);
    TileMapWriteRow(str_cfg_do_or_not, &g_tilemap1[7 * MAP_W + VALUE_COL],
                    0, 8);

    SlotSetPos(CURSOR_SLOT, CURSOR_Z, CURSOR_X,
               g_menu->row * ROW_PITCH + CURSOR_Y0);
    ConfigBeginEdit();
    ConfigPlaceMarkers();
    SlotClear(5);
    SlotClear(6);
}
