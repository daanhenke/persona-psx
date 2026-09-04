/* Persona 1 (JP) - S2D overlay @ 0x80068EA0
 *
 * Same routine as the DNG overlay's DngPollInput (see src/p1-jp/dng/step.c),
 * compiled into this overlay against its own globals.
 *
 * `tmp` is load-bearing: reusing one int for the call result and then for 0x1F
 * puts the constant in a register before the pointer load, matching how the
 * original schedules it. Do not "simplify".
 */
#include <types.h>

extern u_short *g_s2d_menu;
extern u_char   g_s2d_menu_blink;
extern u_char   g_s2d_menu_allow_hold;
extern short    g_s2d_menu_subsel;
extern short    g_s2d_menu_sel;

extern int  InputBuildAcceptMasks(u_short *p);
extern void S2dUpdateMenuSprites(int arg);
extern u_char InputCheckAcceptA(int arg);
extern u_char InputCheckAcceptB(int arg);
extern void S2dDrawStatusHud(void);

void S2dPollInput(void)
{
    int   tmp;
    short val;

    if (InputBuildAcceptMasks(g_s2d_menu) != 0) {
        tmp = 0;
        S2dUpdateMenuSprites(tmp);
        g_s2d_menu_blink = 0;
    }

    tmp = InputCheckAcceptA(2) != 0;
    if (tmp) {
        val = *g_s2d_menu;
        g_s2d_menu_subsel = 0;
        tmp = 0x1F;
        g_s2d_menu_blink = tmp;
        val = val + 1;
    } else {
        if (InputCheckAcceptB(2) == 0 && g_s2d_menu_allow_hold == 0) {
            goto out;
        }
        val = 0xFF;
    }
    g_s2d_menu_sel = val;

out:
    S2dDrawStatusHud();
}
