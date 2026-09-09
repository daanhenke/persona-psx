/* Persona 1 (JP) - the config screen's battle page.
 *                      DNG         ADV         S2D
 *   ConfigPlaceMarkers  0x80084FD0  0x80076448  0x80075400
 *   ConfigBeginEdit     0x800850F8  0x80076574  0x8007552C
 *   ConfigApplyOption   0x800851D0  0x8007664C  0x80075604
 *
 * The list page is a unit of its own ahead of this, in configlist.c, and the
 * redraw between them in configbattle.c.
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


void ConfigPlaceMarkers(void)
{
    void *def;

    def = &g_config_marker_def;
    SlotInitTagged(def, 2, 0, 0, 0);
    SlotInitTagged(def, 3, 0, 0, 0);
    SlotInitTagged(def, 4, 0, 0, 0);
    SlotSetFlicker(2, 0);
    SlotSetFlicker(3, 0);
    SlotSetFlicker(4, 0);
    SlotSetFlicker(g_menu->row + 2, 1);
    SlotSetPos(2, MARKER_Z, g_options[1] * OPT_STEP + OPT_X0, 0x48);
    SlotSetPos(3, MARKER_Z, g_options[2] * OPT_STEP + OPT_X0, 0x60);
    SlotSetPos(4, MARKER_Z, g_options[0x23] * OPT_STEP + OPT_X0, 0x78);
}

/* Arms the value list for the battle page's selected row, starting it at that
   option's saved value. Row 3 opens a page of its own and has nothing to
   edit; the count passed with each is one less than the number of choices. */
void ConfigBeginEdit(void)
{
    switch (g_menu->row) {
    case 0:
        MenuListInit(&g_menu->list[1], g_options[1], 0, 1, OPT_FLAGS);
        break;
    case 1:
        MenuListInit(&g_menu->list[1], g_options[2], 0, 2, OPT_FLAGS);
        break;
    case 2:
        MenuListInit(&g_menu->list[1], g_options[0x23], 0, 1, OPT_FLAGS);
        break;
    case 3:
        MenuListInit(&g_menu->list[1], 0, 0, 0, 0);
        break;
    }
}

/* Stores the edited value into whichever option the selected row names. */
void ConfigApplyOption(void)
{
    switch (g_menu->row) {
    case 0:
        g_options[1] = OPT_VALUE;
        break;
    case 1:
        g_options[2] = OPT_VALUE;
        break;
    case 2:
        g_options[0x23] = OPT_VALUE;
        break;
    }
}
