/* Persona 1 (JP) - the config screen.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                      DNG         ADV         S2D
 *   ConfigPlaceMarkers 0x80084C4C  0x800760C0  0x80075078
 *   ConfigBeginEdit    0x800850F8  0x80076574  0x8007552C
 *   ConfigApplyOption  0x800851D0  0x8007664C  0x80075604
 *
 * Each setting is a row of evenly spaced choices with a marker sprite sitting
 * on the current one, so a value is really a column index.
 */
#include <types.h>
#include <persona/common/menuctx.h>

/* Saved option bytes. Index 0 is the one AdvLoadBgm reads to choose between
   the two libsnd output-mode calls. */
extern u_char g_options[];

/* This unit was built against int-taking prototypes, so the slot argument is
   passed unmasked and the callee narrows it. */
extern void SlotSetFlicker(int slot, int on);
extern void SlotSetPos(int slot, int attr, int x, int y);

/* Where a marker sits for value n. */
#define OPT_X0   0xA0
#define OPT_STEP 40

/* The value list is a row of columns: wrap at both ends, Right moves forward,
   and every step clicks. */
#define OPT_FLAGS (MENU_WRAP | MENU_RIGHT_IS_NEXT | MENU_CLICK_A)

/* Stops both markers blinking, arms the one the context points at, then moves
   each to the column its option value selects. */
void ConfigPlaceMarkers(void)
{
    SlotSetFlicker(2, 0);
    SlotSetFlicker(3, 0);
    SlotSetFlicker(g_menu->slot_base + 2, 1);
    SlotSetPos(2, 0x42, g_options[3] * OPT_STEP + OPT_X0, 0x48);
    SlotSetPos(3, 0x42, g_options[0] * OPT_STEP + OPT_X0, 0x60);
}

/* Arms the value list for the row the cursor is on, starting it at that
   option's saved value. Row 3 is the way out and has nothing to edit. */
void ConfigBeginEdit(void)
{
    switch (g_menu->row) {
    case 0:
        MenuListInit(&g_menu->sel.list[0], g_options[1], 0, 1, OPT_FLAGS);
        break;
    case 1:
        MenuListInit(&g_menu->sel.list[0], g_options[2], 0, 2, OPT_FLAGS);
        break;
    case 2:
        MenuListInit(&g_menu->sel.list[0], g_options[0x23], 0, 1, OPT_FLAGS);
        break;
    case 3:
        MenuListInit(&g_menu->sel.list[0], 0, 0, 0, 0);
        break;
    }
}

/* The sound page has its own two settings and picks the row from slot_base,
   the field ConfigPlaceMarkers uses to decide which marker to arm. */
void ConfigSoundBeginEdit(void)
{
    switch (g_menu->slot_base) {
    case 0:
        MenuListInit(&g_menu->sel.list[0], g_options[3], 0, 1, OPT_FLAGS);
        break;
    case 1:
        MenuListInit(&g_menu->sel.list[0], g_options[0], 0, 1, OPT_FLAGS);
        break;
    case 2:
    case 3:
    case 4:
        MenuListInit(&g_menu->sel.list[0], 0, 0, 0, 0);
        break;
    }
}

/* Stores the edited value into whichever option the selected row names. */
void ConfigApplyOption(void)
{
    switch (g_menu->row) {
    case 0:
        g_options[1] = g_menu->sel.value;
        break;
    case 1:
        g_options[2] = g_menu->sel.value;
        break;
    case 2:
        g_options[0x23] = g_menu->sel.value;
        break;
    }
}
