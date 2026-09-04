/* Persona 1 (JP) - DNG overlay @ 0x80078EE8
 *
 * Per-frame step hook. The same routine appears in the S2D and ADV overlays at
 * 0x80068EA0 / 0x80069914, with the same shape and different globals.
 *
 * Callees and globals are not understood yet, so they keep address-derived
 * names; the control flow is the point.
 *
 * `tmp` is load-bearing. Reusing one int for the call result and then for 0x1F
 * is what puts the constant in a register before the pointer load, matching how
 * the original schedules it. Found by decomp-permuter; do not "simplify".
 */
#include <types.h>

extern u_short *g_dng_menu;   /* -> object whose first u16 feeds the store */
extern u_char   g_dng_menu_blink;
extern u_char   g_dng_menu_allow_hold;
extern short    g_dng_menu_subsel;
extern short    g_dng_menu_sel;

extern int  InputBuildAcceptMasks(u_short *p);
extern void DngUpdateMenuSprites(int arg);
extern int  InputCheckAcceptA(int arg);
extern int  InputCheckAcceptB(int arg);
extern void DngDrawStatusHud(void);

void DngPollInput(void)
{
    int   tmp;
    short val;

    if (InputBuildAcceptMasks(g_dng_menu) != 0) {
        tmp = 0;
        DngUpdateMenuSprites(tmp);
        g_dng_menu_blink = 0;
    }

    tmp = InputCheckAcceptA(2) != 0;
    if (tmp) {
        val = *g_dng_menu;
        g_dng_menu_subsel = 0;
        tmp = 0x1F;
        g_dng_menu_blink = tmp;
        val = val + 1;
    } else {
        if (InputCheckAcceptB(2) == 0 && g_dng_menu_allow_hold == 0) {
            goto out;
        }
        val = 0xFF;
    }
    g_dng_menu_sel = val;

out:
    DngDrawStatusHud();
}
