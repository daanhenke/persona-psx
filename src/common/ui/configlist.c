/* Persona 1 (JP) - the config screen's list page.
 *                          DNG         ADV         S2D
 *   ConfigListPlaceMarkers 0x80084C4C  0x800760C0  0x80075078
 *   ConfigListBeginEdit    0x80084CE8  0x8007615C  0x80075114
 *   ConfigListApplyOption  0x80084DA0  0x80076214  0x800751CC
 *
 * The battle page is a unit of its own behind this one, in config.c, and its
 * redraw between the two, in configbattle.c.
 */
/*
 * Each setting is a row of evenly spaced choices with a marker sprite sitting
 * on the current one, so a value is really a column index. The choices are not
 * drawn one at a time: a whole row of them is one string, spaced so a marker
 * parked on any of them lines up, which is why nothing here has to know what
 * the alternatives are.
 *
 * The screen is two pages. The list page offers five entries and shows a value
 * beside the first two:
 *
 *   0  AUTO MAP            FREE / FIXED         g_options[3]
 *   1  SOUND               MONO / STEREO        g_options[0]
 *   2  CONTROLLER SETTINGS (opens a page)
 *   3  WINDOW SETTINGS     (opens a page)
 *   4  BATTLE SETTINGS     (opens a page)
 *
 * and the battle page offers three settings and a fourth entry that opens yet
 * another:
 *
 *   0  COMMAND CONFIRM     DO / DO NOT          g_options[1]
 *   1  MESSAGE SPEED       NORMAL / FAST / OFF  g_options[2]
 *   2  WINDOW ANIMATION    DO / DO NOT          g_options[0x23]
 *   3  AUTO BATTLE SETTINGS (opens a page)
 *
 * The list page picks its row from slot_base and the battle page from row,
 * which is the only structural difference between the two.
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

/* The list page's two markers: stops both blinking, arms the one the context
   points at, then moves each to the column its option value selects. The
   battle page has three, placed by ConfigPlaceMarkers below. */
void ConfigListPlaceMarkers(void)
{
    SlotSetFlicker(2, 0);
    SlotSetFlicker(3, 0);
    SlotSetFlicker(g_menu->slot_base + 2, 1);
    SlotSetPos(2, 0x42, g_options[3] * OPT_STEP + OPT_X0, 0x48);
    SlotSetPos(3, 0x42, g_options[0] * OPT_STEP + OPT_X0, 0x60);
}

/* The same for the list page, which picks its row from slot_base - the field
   ConfigListPlaceMarkers uses to decide which marker to arm. Its last three
   rows open pages rather than holding a value. */
void ConfigListBeginEdit(void)
{
    switch (g_menu->slot_base) {
    case 0:
        MenuListInit(&g_menu->list[1], g_options[3], 0, 1, OPT_FLAGS);
        break;
    case 1:
        MenuListInit(&g_menu->list[1], g_options[0], 0, 1, OPT_FLAGS);
        break;
    case 2:
    case 3:
    case 4:
        MenuListInit(&g_menu->list[1], 0, 0, 0, 0);
        break;
    }
}

/* The list page's apply. Only its first two rows hold a value, and changing
   SOUND takes effect at once rather than waiting for the next track. */
void ConfigListApplyOption(void)
{
    switch (g_menu->slot_base) {
    case 0:
        OPT_AT[3] = OPT_VALUE;
        break;
    case 1:
        OPT_AT[0] = OPT_VALUE;
        if (OPT_AT[0] != 0) {
            SsSetStereo();
        } else {
            SsSetMono();
        }
        break;
    }
}
