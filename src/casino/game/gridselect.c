/* Reveal the selected grid row, then open its controls and object animation. */
#include <persona/casino/casino.h>
extern u_char g_grid_select_palette_on, D_80099B10[], D_800A8700[];
extern short D_800A6748[][298];
extern CasinoLayout *D_800A2BC8[];
extern CasinoLayoutDef grid_choices[] __asm__("D_800A2BC8+0x468");
extern void CasinoStartPalAnim(CasinoPalAnim *);
extern void func_8008A9DC(int, short, short);
extern void func_8008C5DC(CasinoLayout *, short *);
extern void CasinoTween(CasinoLayout *, int, short, short, short, short);
extern void CasinoShowLayout(CasinoLayout *, u_char);
extern void CasinoStartAnim(CasinoObj *, short, short, short, short,
                            long, long, long, long, long, long);
extern void func_8008B428(void), func_8008AA64(int, int);

void func_80088C8C(void)
{
    if (!g_grid_select_palette_on)
        CasinoStartPalAnim((CasinoPalAnim *)(D_80099B10 + 360));
    if (!g_casino_timer) {
        int player;
        func_8008A9DC(1, -128, 32);
        player = D_800A8700[1];
        func_8008C5DC(grid_choices[player & 1].l,
                     D_800A6748[D_800A8700[2]] + 1 + player * 33);
        CasinoTween(grid_choices[D_800A8700[1] & 1].l, -160, 0, 0, 0, 32);
    }
    if (g_casino_timer == 32) {
        CasinoShowLayout(grid_choices[!(D_800A8700[1] & 1)].l, 0);
        CasinoTween(D_800A2BC8[56], -24, -24, 48, 48, 12);
        CasinoStartAnim((CasinoObj *)D_800A2BC8[851], 16, 2048, 0, 0,
                        0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer >= 44) {
        func_8008B428();
        func_8008AA64(D_800A8700[1], 1);
        g_casino_step = 112;
        g_casino_timer = -1;
    }
}
