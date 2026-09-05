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

extern u_short *g_menu;   /* -> object whose first u16 feeds the store */
extern u_char   g_menu_blink;
extern u_char   g_menu_allow_hold;
extern short    g_menu_subsel;
extern short    g_menu_sel;

extern int  InputBuildAcceptMasks(u_short *p);
extern void UpdateMenuSprites(int arg);
extern int  InputCheckAcceptA(int arg);
extern int  InputCheckAcceptB(int arg);
extern void DrawStatusHud(void);

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
