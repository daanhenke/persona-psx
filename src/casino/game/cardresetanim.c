#include <persona/casino/blackjack.h>

extern u_char D_800B0AF8[];
extern void CasinoSpritesSetOn(int, int, int);
extern void CasinoStartAnim(CasinoObj *, short, short, short, short,
                            long, long, long, long, long, long);

void func_800808D8(void)
{
    int i;
    if (g_casino_timer == 0)
    {
        for (i = 24; i >= 13; i--)
            CasinoStartAnim(&g_casino_objs[i], 32, 0, 0, 0, 320, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 32)
    {
        for (i = 24; i >= 13; i--)
            CasinoStartAnim(&g_casino_objs[i], 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer >= 65)
    {
        CasinoSpritesSetOn(0, 140, 1);
        CasinoSpritesSetOn(28, 2, 0);
        CasinoSpritesSetOn(38, 2, 0);
        g_casino_step = 38;
        g_casino_timer = -1;
        D_800B0AF8[1] = 0;
        g_bj_card_obj += 12;
        g_bj_shoe_pos -= 12;
        g_bj_count[0] -= 10;
        g_bj_dealer_count -= 2;
    }
}
