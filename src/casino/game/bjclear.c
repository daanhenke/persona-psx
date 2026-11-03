/* Blackjack hand cleanup and presentation reset. */
extern void *D_8009ACE4[], *D_8009AD88[], *D_8009B2B4[], *D_8009B6D8[], *D_8009B91C[], *D_8009B920;
extern char D_8009B7B0[], g_casino_objs[];
extern int D_8009C280[], D_8009C290[], D_8009C428[], D_8009C564[];
extern unsigned char D_800A8818, g_bj_card_obj, g_bj_count, g_bj_label_n[], g_casino_bet,
    g_casino_step;
extern short g_casino_seqs[], g_casino_main_vab;
extern int g_casino_timer;
extern void CasinoBuildLayouts(void *, int), CasinoPlaySeq(short *, unsigned int, short),
    CasinoQueueCluts(int, int, int), CasinoSpritesSetOn(int, int, int),
    CasinoStartAnim(void *, int, int, int, int, int, int, int, int, int, int),
    CasinoTween(void *, short, short, short, short, short), func_8007E614(int),
    func_8007EE78(void *, int, int);
void func_8007A9F8(void)
{
    int i;
    if (g_casino_timer == 0)
    {
        CasinoPlaySeq(g_casino_seqs, 0x1387D0, g_casino_main_vab);
        for (i = 24; i > g_bj_card_obj; i--)
            CasinoStartAnim(g_casino_objs + i * 16, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
        if (g_bj_count >= 3U)
        {
            CasinoSpritesSetOn(0x3F, 3, 1);
            CasinoSpritesSetOn(0x49, 3, 1);
            CasinoSpritesSetOn(0x53, 3, 1);
            CasinoSpritesSetOn(0x5D, 3, 1);
        }
        func_8007EE78(D_8009ACE4, -g_bj_label_n[0], 8);
        CasinoTween(D_8009B2B4[1], 8, 0, -0x10, 0, 2);
        func_8007EE78(D_8009AD88, -g_bj_label_n[1], 8);
        CasinoTween(D_8009B2B4[3], 8, 0, -0x10, 0, 2);
        g_casino_bet = 0;
        func_8007E614(0);
        CasinoQueueCluts(0x2EC, 7, 0x7D64);
        CasinoQueueCluts(0x2F3, 7, 0x7D64);
        if (D_800A8818 == 1)
        {
            CasinoTween(D_8009B920, 0x28, 0, 0, 0, 4);
        }
    }
    if (g_casino_timer == 8)
    {
        CasinoSpritesSetOn(0x368, 1, 0);
        CasinoTween(D_8009AD88[1], 0, 8, 0, -0x10, 8);
        CasinoTween(D_8009AD88[3], 0, 8, 0, -0x10, 8);
        CasinoTween(D_8009ACE4[1], 0, 8, 0, -0x10, 8);
        CasinoTween(D_8009ACE4[3], 0, 8, 0, -0x10, 8);
    }
    if (g_casino_timer >= 0x21)
    {
        *(int *)(D_8009C280[0] + 8) = 0x13C;
        *(int *)(D_8009C290[0] + 8) = -0x144;
        *(int *)(D_8009C428[0] + 8) = -8;
        *(short *)(D_8009C428[0] + 2) = 0x800;
        *(int *)(D_8009C564[0] + 8) = -8;
        *(short *)(D_8009C564[0] + 2) = 0x800;
        CasinoSpritesSetOn(0x2AD, 3, 0);
        CasinoSpritesSetOn(0x2B9, 0xA, 0);
        CasinoSpritesSetOn(0x2B0, 3, 0);
        CasinoSpritesSetOn(0x2C3, 0xA, 0);
        if (D_800A8818 == 1)
        {
            CasinoBuildLayouts(D_8009B91C, 1);
        }
        CasinoTween(D_8009B6D8[1], 0x140, 0, 0, 0, 0x20);
        CasinoStartAnim(D_8009B7B0, 0x20, 0, 0, 0, -0x140, 0, 0, 0, 0, 0);
        g_casino_timer = -1;
        g_casino_step = 0x12;
    }
}
