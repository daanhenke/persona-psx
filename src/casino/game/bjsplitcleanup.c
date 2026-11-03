#include <persona/casino/blackjack.h>
extern CasinoPalAnim   D_80099990[];
extern CasinoLayoutDef D_8009AE2C[], D_8009AED0[], D_8009B91C[];
extern CasinoLayout *  D_8009B928, *D_8009B930;
extern CasinoObj       D_8009C294[2], D_8009C2B4[2], D_8009C42C, D_8009C43C, D_8009C568, D_8009C578;
extern unsigned char   D_800A8768[2], D_800A8818[2], g_bj_result_palette_on;
extern void            CasinoStartPalAnim(CasinoPalAnim*), CasinoBuildLayouts(CasinoLayoutDef*, int);
extern void            CasinoPlaySeq(short*, u_long*, short), CasinoQueueCluts(int, int, int), CasinoSpritesSetOn(short, short, int);
extern void            CasinoStartAnim(CasinoObj*, short, short, short, short, long, long, long, long, long, long);
extern void            CasinoTween();
extern void            func_8007E614(unsigned char), func_8007EE78(void*, int, int);
void                   func_8007CBE4(void)
{
    int i, timer;
    if (g_bj_result_palette_on == 0)
    {
        CasinoStartPalAnim(D_80099990);
    }
    if (g_casino_timer == 0)
    {
        g_casino_bet = 0;
        CasinoPlaySeq(g_casino_seqs, (u_long*)0x1387D0, g_casino_main_vab);
        for (i = 24; i > g_bj_card_obj; i--)
            CasinoStartAnim(&g_casino_objs[i], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
        if ((unsigned char)g_bj_count[0] >= 3U)
        {
            CasinoSpritesSetOn(0x3C, 3, 1);
            CasinoSpritesSetOn(0x46, 3, 1);
            CasinoSpritesSetOn(0x50, 3, 1);
            CasinoSpritesSetOn(0x5A, 3, 1);
        }
        if ((unsigned char)g_bj_count[1] >= 3U)
        {
            CasinoSpritesSetOn(0x43, 3, 1);
            CasinoSpritesSetOn(0x4D, 3, 1);
            CasinoSpritesSetOn(0x57, 3, 1);
            CasinoSpritesSetOn(0x61, 3, 1);
        }
        func_8007EE78(D_8009ACE4, -(int)g_bj_label_n[0], 8);
        CasinoTween(D_8009B2B4[0].l, 8, 0, -0x10, 0, 2);
        func_8007EE78(D_8009AE2C, -(int)g_bj_label_n[1], 8);
        CasinoTween(D_8009B2B4[2].l, 8, 0, -0x10, 0, 2);
        func_8007EE78(D_8009AED0, -(int)g_bj_label_n[2], 8);
        CasinoTween(D_8009B2B4[3].l, 8, 0, -0x10, 0, 2);
        func_8007E614(g_casino_bet);
        CasinoQueueCluts(0x2EC, 7, 0x7D64);
        CasinoQueueCluts(0x2F3, 7, 0x7D64);
        switch (D_800A8768[0])
        {
            case 11:
                CasinoStartAnim(&D_8009C294[0], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
                CasinoStartAnim(&D_8009C294[1], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
                break;
            case 12:
                CasinoStartAnim(&D_8009C42C, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
                break;
            case 13:
                CasinoStartAnim(&D_8009C568, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
                break;
        }
        switch (D_800A8768[1])
        {
            case 11:
                CasinoStartAnim(&D_8009C2B4[0], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
                CasinoStartAnim(&D_8009C2B4[1], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
                break;
            case 12:
                CasinoStartAnim(&D_8009C43C, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
                break;
            case 13:
                CasinoStartAnim(&D_8009C578, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
                break;
        }
        if (D_800A8818[0] == 1)
        {
            CasinoTween(D_8009B928, -0x140, 0, 0, 0, 0x20);
        }
        if (D_800A8818[1] == 1)
        {
            CasinoTween(D_8009B930, -0x140, 0, 0, 0, 0x20);
        }
    }
    timer = g_casino_timer;
    if (timer == 8)
    {
        CasinoTween(D_8009ACE4[0].l, 0, 8, 0, -0x10, timer);
        CasinoTween(D_8009ACE4[1].l, 0, 8, 0, -0x10, timer);
        CasinoTween(D_8009AE2C[0].l, 0, 8, 0, -0x10, timer);
        CasinoTween(D_8009AE2C[1].l, 0, 8, 0, -0x10, timer);
        CasinoTween(D_8009AED0[0].l, 0, 8, 0, -0x10, timer);
        CasinoTween(D_8009AED0[1].l, 0, 8, 0, -0x10, timer);
    }
    if (g_casino_timer == 0x20)
    {
        if (D_800A8818[0] == 1)
        {
            CasinoSpritesSetOn(0x369, 1, 0);
        }
        if (D_800A8818[1] == 1)
        {
            CasinoSpritesSetOn(0x36A, 1, 0);
        }
        CasinoBuildLayouts(D_8009B91C, 1);
        D_800A8818[0] = 0U;
        D_800A8818[1] = 0U;
    }
    if (g_casino_timer >= 0x21)
    {
        switch (D_800A8768[0])
        {
            case 11:
                D_8009C294[0].xform->trans.vx = 240;
                D_8009C294[1].xform->trans.vx = -400;
                break;
            case 12:
                D_8009C42C.xform->trans.vx = -80;
                D_8009C42C.xform->rot.vy   = 2048;
                break;
            case 13:
                D_8009C568.xform->trans.vx = -80;
                D_8009C568.xform->rot.vy   = 2048;
                break;
        }
        switch (D_800A8768[1])
        {
            case 11:
                D_8009C2B4[0].xform->trans.vx = 400;
                D_8009C2B4[1].xform->trans.vx = -240;
                break;
            case 12:
                D_8009C43C.xform->trans.vx = 80;
                D_8009C43C.xform->rot.vy   = 2048;
                break;
            case 13:
                D_8009C578.xform->trans.vx = 80;
                D_8009C578.xform->rot.vy   = 2048;
                break;
        }
        CasinoSpritesSetOn(0x2B6, 3, 0);
        CasinoSpritesSetOn(0x2B3, 3, 0);
        CasinoSpritesSetOn(0x2CD, 0xA, 0);
        CasinoSpritesSetOn(0x2D7, 0xA, 0);
        CasinoTween(D_8009B6D8[0].l, 0x140, 0, 0, 0, 0x20);
        CasinoStartAnim(D_8009B7B0, 0x20, 0, 0, 0, -0x140, 0, 0, 0, 0, 0);
        g_casino_timer = -1;
        g_casino_step  = 0x12;
    }
}
