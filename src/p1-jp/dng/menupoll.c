/* Persona 1 (JP) - menu input poll, DNG's copy.  DNG @ 0x80078EE8
 *
 * The same routine as src/p1-jp/common/menupoll.c, which covers the S2D and ADV
 * copies. DNG's is two instructions shorter: it was built against a prototype
 * for InputCheckAcceptA/B returning int, so the u_char result masking the other
 * two carry is absent. That is the only difference, and it is why this cannot
 * share their source.
 *
 * `tmp` is load-bearing. Reusing one int for the call result and then for 0x1F
 * is what puts the constant in a register before the pointer load, matching how
 * the original schedules it. Found by decomp-permuter; do not "simplify".
 */
#include <types.h>
#include <persona/common/menu.h>

/* The int return is this source's half of the difference described above, so
   these two stay here rather than in persona/common/menu.h. */
extern int InputCheckAcceptA(int arg);
extern int InputCheckAcceptB(int arg);

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
