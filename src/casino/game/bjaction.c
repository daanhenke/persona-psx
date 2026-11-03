extern unsigned int g_casino_pad_trig, g_casino_money;
extern unsigned char D_800A8674, g_bj_count[], g_bj_play_step, D_800A8818[], D_800B09F4[],
    g_casino_jackpot_add, g_casino_bet, g_bj_natural[], D_800AAEA8[], g_bj_label_n[], g_bj_total[],
    g_bj_split_total[];
extern short g_casino_seqs[], g_casino_main_vab;
extern int g_casino_timer;
extern void *D_8009B928, *D_8009B930, *D_8009B2B4[];
extern char D_8009AE2C[], D_8009AED0[];
extern void CasinoPlaySeq(short *, int, short), CasinoSpritesSetOn(int, int, int),
    CasinoTween(void *, short, short, short, short, short), SsSeqPlay(short, char, short),
    CasinoPlayStepAnim(int, int), func_8007F368(int, int),
    func_8007EB20(unsigned char, unsigned char, void *, void *, void *, unsigned char, short);
void func_80081934(unsigned char can_double)
{
    if ((g_casino_pad_trig & 16) && can_double && g_bj_count[D_800A8674] == 2)
    {
        CasinoPlaySeq(&g_casino_seqs[3], 0x138794, g_casino_main_vab);
        CasinoSpritesSetOn(D_800A8674 + 873, 1, 1);
        if (D_800A8674 == 0)
            CasinoTween(D_8009B928, -320, 0, 0, 0, 32);
        else if (D_800A8674 == 1)
            CasinoTween(D_8009B930, 320, 0, 0, 0, 32);
        g_bj_play_step = 112;
        D_800A8818[D_800A8674] = 1;
        g_casino_jackpot_add = D_800B09F4[D_800A8674];
        g_casino_money -= D_800B09F4[D_800A8674];
        D_800B09F4[D_800A8674] *= 2;
        g_casino_bet = D_800B09F4[D_800A8674];
    }
    else if (g_casino_pad_trig & 64)
        g_bj_play_step = 113;
    else if (g_casino_pad_trig & 32)
        g_bj_play_step = 112;
    if (g_bj_play_step != 27)
    {
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        g_casino_timer = -1;
        if (g_bj_count[D_800A8674] == 2 && can_double)
        {
            CasinoPlayStepAnim(0, 0);
            CasinoPlayStepAnim(0, 2);
            CasinoPlayStepAnim(0, 4);
        }
        else
        {
            func_8007F368(0, 0);
            func_8007F368(0, 2);
        }
        switch (D_800A8674)
        {
        case 0:
            if (g_bj_natural[0] == 10 && !D_800AAEA8[4])
                func_8007EB20(10, g_bj_total[1], D_8009B2B4[5], &g_bj_label_n[1], D_8009AE2C, 1, 1);
            break;
        case 1:
            if (g_bj_natural[1] == 10 && !D_800AAEA8[4])
                func_8007EB20(10, g_bj_split_total[1], D_8009B2B4[7], &g_bj_label_n[2], D_8009AED0,
                              1, 1);
            break;
        }
    }
}
