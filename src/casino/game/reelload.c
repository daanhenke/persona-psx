/* Load the reel-game graphics in stages and initialize the two reel strips. */
#include <persona/casino/casino.h>
extern CasinoLayoutDef D_800A1A54[], D_800A20EC[];
extern u_char *D_800B3E28[], *D_800B3E3C[];
extern s8 D_800AF834[];
extern char D_8009E8E8[];
extern void CasinoGame3LoadVram0(void), CasinoGame3LoadVram1(void), CasinoGame3LoadVram2(void);
extern void CasinoGame3LoadVram3(void), CasinoGame3LoadVram4(void), CasinoGame3LoadVram5(void);
extern void CasinoGame3LoadSound(void), func_800853F4(void), func_8008514C(void);
extern void func_80087C98(void *);
extern void CasinoQueueSemi(short, short, u_char);

void func_800829F4(void)
{
    int i;
    if (g_casino_timer == 0) CasinoGame3LoadVram0();
    if (g_casino_timer == 1) CasinoGame3LoadVram1();
    if (g_casino_timer == 2) CasinoGame3LoadVram2();
    if (g_casino_timer == 3) CasinoGame3LoadVram3();
    if (g_casino_timer == 4) CasinoGame3LoadVram4();
    if (g_casino_timer == 5) CasinoGame3LoadVram5();
    if (g_casino_timer == 6) {
        CasinoGame3LoadSound();
        func_800853F4();
        for (i = 0; i < 10; i += 2) {
            *D_800A1A54[i].l->tex_idx = D_800B3E28[i / 2][D_800AF834[344 + i / 2]] + 8;
            *D_800A1A54[i + 1].l->tex_idx = D_800B3E28[i / 2][D_800AF834[344 + i / 2]];
        }
        for (i = 0; i < 32; i += 2) {
            *D_800A20EC[i].l->tex_idx = D_800B3E3C[i / 2][D_800AF834[349 + i / 2]] + 8;
            *D_800A20EC[i + 1].l->tex_idx = D_800B3E3C[i / 2][D_800AF834[349 + i / 2]];
        }
        func_8008514C();
        CasinoFade(0, 1000, 128, 128, 128, 32);
        func_80087C98(D_8009E8E8);
        CasinoQueueSemi(516, 5, 1);
        CasinoQueueSemi(521, 16, 1);
    }
    if (g_casino_timer >= 39) {
        g_casino_timer = -1;
        g_casino_step = 18;
    }
}
