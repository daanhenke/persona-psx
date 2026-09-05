/* Persona 1 (JP) - the config screen.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                      DNG         ADV         S2D
 *   ConfigPlaceMarkers 0x80084C4C  0x800760C0  0x80075078
 *   ConfigApplyOption  0x800851D0  0x8007664C  0x80075604
 *
 * Each setting is a row of evenly spaced choices with a marker sprite sitting
 * on the current one, so a value is really a column index.
 */
#include <types.h>

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
    /* 0x3D0 */ u_char value;
} MenuCtx;   /* g_menu points at one of these */

extern MenuCtx *g_menu;

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

/* Stores the edited value into whichever option the selected row names. */
void ConfigApplyOption(void)
{
    switch (g_menu->row) {
    case 0:
        g_options[1] = g_menu->value;
        break;
    case 1:
        g_options[2] = g_menu->value;
        break;
    case 2:
        g_options[0x23] = g_menu->value;
        break;
    }
}
