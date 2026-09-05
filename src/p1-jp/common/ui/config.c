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
#include <persona/common/menulist.h>

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

typedef struct {
    /* 0x000 */ u_char pad000[0x1C0];
    /* 0x1C0 */ int    slot_base;
    /* 0x1C4 */ u_char pad1C4[0xC];
    /* 0x1D0 */ int    row;
    /* 0x1D4 */ u_char pad1D4[0x1FC];
    /* 0x3D0 */ union {
        MenuList list;
        u_char   value;   /* the low byte of list.cur, which is all an option
                             value ever needs */
    } opt;
} MenuCtx;   /* g_menu points at one of these */

extern MenuCtx *g_menu;

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
        MenuListInit(&g_menu->opt.list, g_options[1], 0, 1, OPT_FLAGS);
        break;
    case 1:
        MenuListInit(&g_menu->opt.list, g_options[2], 0, 2, OPT_FLAGS);
        break;
    case 2:
        MenuListInit(&g_menu->opt.list, g_options[0x23], 0, 1, OPT_FLAGS);
        break;
    case 3:
        MenuListInit(&g_menu->opt.list, 0, 0, 0, 0);
        break;
    }
}

/* Stores the edited value into whichever option the selected row names. */
void ConfigApplyOption(void)
{
    switch (g_menu->row) {
    case 0:
        g_options[1] = g_menu->opt.value;
        break;
    case 1:
        g_options[2] = g_menu->opt.value;
        break;
    case 2:
        g_options[0x23] = g_menu->opt.value;
        break;
    }
}
