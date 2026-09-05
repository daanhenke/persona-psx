/* Persona 1 (JP) - menu input poll.
 *
 * Compiled into more than one overlay rather than called across the boundary:
 *   S2D @ 0x80068EA0
 *   ADV @ 0x80069914
 * Byte-identical once call targets and global addresses are masked out.
 *
 * DNG has the same routine at 0x80078EE8, but two instructions shorter - it was
 * built against a prototype returning int where these two return u_char, so the
 * result masking is absent. It keeps its own source, src/p1-jp/dng/menupoll.c.
 *
 * `tmp` is load-bearing. Reusing one int for the call result and then for 0x1F
 * is what puts the constant in a register before the pointer load, matching how
 * the original schedules it. Found by decomp-permuter; do not "simplify".
 */
#include <types.h>
#include <persona/common/menu.h>

/* The u_char return is this source's half of the difference described above,
   so these two stay here rather than in persona/common/menu.h. */
extern u_char InputCheckAcceptA(int arg);
extern u_char InputCheckAcceptB(int arg);

void MenuPollInput(void)
{
    int   tmp;
    short val;

    if (InputBuildAcceptMasks(g_menu) != 0) {
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
