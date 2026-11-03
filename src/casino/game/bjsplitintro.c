/* Persona JP1 blackjack hand introduction. */
extern int g_casino_timer;
extern unsigned char g_bj_cards[], g_bj_count[], g_bj_split_total[], g_bj_natural[], g_bj_label_n[],
    D_800A8768[];
extern char g_casino_objs[], D_8009AED0[];
extern void *D_8009AED4, *D_8009AEDC, *D_8009B2B4[];
extern short g_casino_seqs[], g_casino_main_vab;
extern void func_8007E878(int, int, void *, void *), func_8008205C(void *, void *),
    CasinoStartAnim(void *, int, int, int, int, int, int, int, int, int, int),
    CasinoPlaySeq(short *, unsigned int, short), CasinoSpritesSetOn(int, int, int),
    CasinoTween(void *, short, short, short, short, short), CasinoObjSetOn(void *, int, int, int),
    func_8007EB20(int, int, void *, void *, void *, int, int), func_8007FBD8(int);
extern int func_80081DC0(void *);
void func_8007B4DC(void)
{
    if (!g_casino_timer)
    {
        func_8007E878(2, 0, g_bj_cards + 8, g_bj_count + 1);
        func_8008205C(g_bj_cards + 8, g_bj_split_total);
        g_bj_natural[1] = func_80081DC0(g_bj_cards + 8);
        if (g_bj_natural[1] == 8)
            g_bj_natural[1] = 1;
    }
    if (g_casino_timer == 32)
    {
        CasinoPlaySeq(g_casino_seqs, 0x138830, g_casino_main_vab);
        CasinoStartAnim(g_casino_objs + 304, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 64)
    {
        CasinoStartAnim(g_casino_objs + 304, 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
        CasinoSpritesSetOn(694, 3, 1);
        CasinoTween(D_8009AED4, 0, -8, 0, 16, 8);
        CasinoTween(D_8009AEDC, 0, -8, 0, 16, 8);
    }
    if (g_casino_timer == 72)
    {
        CasinoObjSetOn(g_casino_objs + 352, 2, 17, 0);
        CasinoSpritesSetOn(727, 10, 1);
        func_8007EB20(g_bj_natural[1], g_bj_split_total[1], D_8009B2B4[7], g_bj_label_n + 2,
                      D_8009AED0, 1, 8);
    }
    if (g_casino_timer == 96)
    {
        if (g_bj_natural[1] != 1)
        {
            D_800A8768[1] = 113;
            func_8007FBD8(1);
        }
        else if (g_bj_split_total[1] == 21)
            D_800A8768[1] = 113;
    }
}
