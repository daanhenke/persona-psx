/* Persona 1 (JP) - the config screen's three option markers.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x80084FD0   ADV @ 0x8007644C   S2D @ 0x80075404
 *
 * Each of the screen's first three rows is a row of evenly spaced choices with
 * a marker sprite sitting on the current one, so an option's value is really a
 * column index. This builds all three markers, stops them blinking, arms the
 * one on the row the cursor is on, and slides each to its option's column.
 *
 * The three options are the same ones ConfigBeginEdit maps from g_menu->row;
 * the sound page has a separate pair, in src/p1-jp/common/ui/config.c.
 */
#include <types.h>
#include <persona/common/menuctx.h>

extern u_char g_options[];
extern void   g_config_marker_def;

extern void SlotInitTagged(void *def, u_char slot, int attr, short x, short y);

/* This unit was built against int-taking prototypes, so the slot argument is
   passed unmasked and the callee narrows it. */
extern void SlotSetFlicker(int slot, int on);
extern void SlotSetPos(int slot, int attr, int x, int y);

/* Where a marker sits for value n. */
#define OPT_X0   0xA0
#define OPT_STEP 40

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
