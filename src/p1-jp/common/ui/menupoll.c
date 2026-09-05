/* Persona 1 (JP) - menu input poll.
 *
 * Compiled into more than one overlay rather than called across the boundary:
 *   S2D @ 0x80068EA0
 *   ADV @ 0x80069914
 * Byte-identical once call targets and global addresses are masked out.
 *
 * DNG has the same routine at 0x80078EE8, but two instructions shorter - it was
 * built against a prototype returning int where these two return u_char, so the
 * result masking is absent. It keeps its own source,
 * src/p1-jp/dng/ui/menupoll.c.
 */
#include <types.h>
#include <persona/common/menu.h>

/* The u_char return is this source's half of the difference described above,
   so these two stay here rather than in persona/common/menu.h. */
extern u_char InputCheckAcceptA(int arg);
extern u_char InputCheckAcceptB(int arg);

/* One frame of the open menu. Moving the cursor stops the blink and redraws
   the sprites; accept publishes the highlighted entry as `index + 1` in
   g_menu_sel, restarts the blink for 0x1F frames and
   drops the phase back to 0 so MenuTick rebuilds the screen; cancel publishes
   0xFF instead. The status HUD is redrawn either way.
 *
 * Reusing `tmp` for both the call result and 0x1F looks pointless but has to
 * stay; splitting it into two locals breaks the match. */
void MenuPollInput(void)
{
    int   tmp;
    short val;

    if (MenuStepCursor(g_menu) != 0) {
        tmp = 0;
        UpdateMenuSprites(tmp);
        g_menu_blink = 0;
    }

    tmp = InputCheckAcceptA(2) != 0;
    if (tmp) {
        val = *g_menu;
        g_menu_subsel = 0;
        tmp = 0x1F;
        g_menu_blink = tmp;
        val = val + 1;
    } else {
        if (InputCheckAcceptB(2) == 0 && g_menu_allow_hold == 0) {
            goto out;
        }
        val = 0xFF;
    }
    g_menu_sel = val;

out:
    DrawStatusHud();
}
