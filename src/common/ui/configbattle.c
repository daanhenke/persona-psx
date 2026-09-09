/* Persona 1 (JP) - redrawing the config screen's battle page.
 *   ADV 0x800762A0   S2D 0x80075258
 *
 * A unit of its own: DNG's copy is eight bytes longer and is not this source,
 * so that overlay still takes it from asm. The list page ahead of it is in
 * configlist.c and the battle page's own markers behind it in config.c.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/common/menuctx.h>

/* Saved option bytes, read all over the game: index 0 is the one AdvLoadBgm
   reads to choose between the two libsnd output-mode calls, which is the same
   MONO / STEREO setting the list page shows. */
extern u_char g_options[];

/* This unit was built against int-taking prototypes, so the slot argument is
   passed unmasked and the callee narrows it. */
extern void SlotSetFlicker(int slot, int on);
extern void SlotSetPos(u_char slot, int attr, short x, short y);

/* Where a marker sits for value n. */
#define OPT_X0   0xA0
#define OPT_STEP 40

/* The value list is a row of columns: wrap at both ends, Right moves forward,
   and every step clicks. */
#define OPT_FLAGS (MENU_WRAP | MENU_RIGHT_IS_NEXT | MENU_CLICK_A)

/* This routine reaches the option bytes by address rather than through
   g_options. */
#define OPT_AT ((u_char *)0x801F2AC8)

/* An option value never needs more than the low byte of the list index. */
#define OPT_VALUE (*(u_char *)&g_menu->list[1].cur)

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
extern void SlotClear(u_char slot);
extern void TileMapWriteRow(const u_char *src, short *dst, u_short base,
                            u_short count);
extern void ConfigBeginEdit(void);
extern void ConfigPlaceMarkers(void);
#define CURSOR_SLOT 1
#define CURSOR_Z    0x42
#define CURSOR_X    0x48
#define CURSOR_Y0   0x48
#define ROW_PITCH   0x18
#define VALUE_COL 11
extern void   g_config_marker_def;
extern void SlotInitTagged(void *def, u_char slot, int attr, short x, short y);
#define MARKER_Z 0x42


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
