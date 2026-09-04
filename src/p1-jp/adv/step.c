/* Persona 1 (JP) - ADV overlay @ 0x80069914
 *
 * Same routine as the DNG overlay's DngPollInput (see src/p1-jp/dng/step.c),
 * compiled into this overlay against its own globals.
 *
 * `tmp` is load-bearing: reusing one int for the call result and then for 0x1F
 * puts the constant in a register before the pointer load, matching how the
 * original schedules it. Do not "simplify".
 */
#include <types.h>

extern u_short *g_adv_menu;
extern u_char   g_adv_menu_blink;
extern u_char   g_adv_menu_allow_hold;
extern short    g_adv_menu_subsel;
extern short    g_adv_menu_sel;

extern int  InputBuildAcceptMasks(u_short *p);
extern void AdvUpdateMenuSprites(int arg);
extern u_char InputCheckAcceptA(int arg);
extern u_char InputCheckAcceptB(int arg);
extern void AdvDrawStatusHud(void);

void AdvPollInput(void)
{
    int   tmp;
    short val;

    if (InputBuildAcceptMasks(g_adv_menu) != 0) {
        tmp = 0;
        AdvUpdateMenuSprites(tmp);
        g_adv_menu_blink = 0;
    }

    tmp = InputCheckAcceptA(2) != 0;
    if (tmp) {
        val = *g_adv_menu;
        g_adv_menu_subsel = 0;
        tmp = 0x1F;
        g_adv_menu_blink = tmp;
        val = val + 1;
    } else {
        if (InputCheckAcceptB(2) == 0 && g_adv_menu_allow_hold == 0) {
            goto out;
        }
        val = 0xFF;
    }
    g_adv_menu_sel = val;

out:
    AdvDrawStatusHud();
}
