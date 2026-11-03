/* Persona JP1 blackjack hand introduction. */
extern int g_casino_timer;
extern unsigned char g_bj_cards[], g_bj_count[], g_bj_total[], g_bj_natural[], g_bj_label_n[],
    D_800A8768;
extern char g_casino_objs[], D_8009AE2C[];
extern void *D_8009AE30, *D_8009AE38, *D_8009B2B4[];
extern short g_casino_seqs[], g_casino_main_vab;
extern void func_8007E878(int, int, void *, void *), func_8008205C(void *, void *),
    CasinoStartAnim(void *, int, int, int, int, int, int, int, int, int, int),
    CasinoPlaySeq(short *, unsigned int, short), CasinoSpritesSetOn(int, int, int),
    CasinoTween(void *, short, short, short, short, short), CasinoObjSetOn(void *, int, int, int),
    func_8007EB20(int, int, void *, void *, void *, int, int), func_8007FBD8(int);
extern int func_80081DC0(void *);
void func_8007B288(void)
{
    if (!g_casino_timer)
    {
        func_8007E878(1, 0, g_bj_cards, g_bj_count);
        func_8008205C(g_bj_cards, g_bj_total);
        g_bj_natural[0] = func_80081DC0(g_bj_cards);
        if (g_bj_natural[0] == 8)
            g_bj_natural[0] = 1;
        CasinoStartAnim(g_casino_objs + 320, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 32)
    {
        CasinoPlaySeq(g_casino_seqs, 0x138830, g_casino_main_vab);
        CasinoStartAnim(g_casino_objs + 320, 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
        CasinoSpritesSetOn(691, 3, 1);
        CasinoTween(D_8009AE30, 0, -8, 0, 16, 8);
        CasinoTween(D_8009AE38, 0, -8, 0, 16, 8);
    }
    if (g_casino_timer == 40)
    {
        CasinoObjSetOn(g_casino_objs + 384, 2, 17, 0);
        CasinoSpritesSetOn(717, 10, 1);
        func_8007EB20(g_bj_natural[0], g_bj_total[1], D_8009B2B4[5], g_bj_label_n + 1, D_8009AE2C,
                      1, 8);
    }
    if (g_casino_timer == 96)
    {
        if (g_bj_natural[0] != 1)
        {
            D_800A8768 = 113;
            func_8007FBD8(0);
        }
        else if (g_bj_total[1] == 21)
            D_800A8768 = 113;
    }
}
