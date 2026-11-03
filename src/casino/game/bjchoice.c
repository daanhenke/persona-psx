/* Persona JP1 blackjack split/double/hit/stand choice handling. */
extern unsigned int g_casino_pad_trig, g_casino_money;
extern unsigned char g_casino_step, g_bj_play_step, g_bj_label_n[], g_casino_bet,
    g_casino_jackpot_add, D_800A8818[];
extern int g_casino_timer;
extern char g_casino_objs[];
extern void *D_8009AD88[], *D_8009B2B4[], *D_8009B920;
extern short g_casino_seqs[], g_casino_main_vab, D_800999A8[];
extern unsigned short GetClut(short, short);
extern void CasinoObjSetOn(void *, int, int, int), CasinoTween(void *, int, int, int, int, int),
    CasinoPlaySeq(short *, unsigned int, short), CasinoSpritesSetOn(int, int, int),
    CasinoQueueCluts(int, int, unsigned short), SsSeqStop(short), SsSeqPlay(short, char, short),
    func_8007F1D0(int, int), CasinoPlayStepAnim(int, int), func_8007F368(int, int);
void func_80081224(unsigned char split, unsigned char extra)
{
    int n;
    if ((g_casino_pad_trig & 128) && split == 1)
    {
        g_casino_step = 119;
        g_bj_play_step = 18;
        CasinoObjSetOn(g_casino_objs + 384, 2, 17, 1);
        n = -g_bj_label_n[1];
        CasinoTween(D_8009AD88[1], -(n * 8), 0, 0, 0, 8);
        CasinoTween(D_8009AD88[3], n * 8, 0, 0, 0, 8);
        CasinoTween(D_8009AD88[5], -(n * 8), 0, n * 16, 0, 8);
        CasinoTween(D_8009B2B4[3], 8, 0, -16, 0, 8);
    }
    else if ((g_casino_pad_trig & 16) && g_casino_money >= g_casino_bet)
    {
        g_casino_money -= g_casino_bet;
        g_casino_jackpot_add = g_casino_bet;
        g_casino_bet *= 2;
        CasinoPlaySeq(g_casino_seqs + 3, 0x138794, g_casino_main_vab);
        CasinoSpritesSetOn(872, 1, 1);
        CasinoTween(D_8009B920, 320, 0, 0, 0, 32);
        CasinoQueueCluts(872, 1, GetClut(D_800999A8[4], D_800999A8[5]));
        g_casino_step = 118;
        g_bj_play_step = 112;
        D_800A8818[0] = 1;
    }
    else if (g_casino_pad_trig & 64)
    {
        g_casino_step = 118;
        g_bj_play_step = 113;
    }
    else if (g_casino_pad_trig & 32)
    {
        g_casino_step = 118;
        g_bj_play_step = 112;
    }
    if (g_casino_step != 27)
    {
        SsSeqStop(g_casino_seqs[4]);
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        g_casino_timer = -1;
        if (split)
        {
            func_8007F1D0(0, 0);
            func_8007F1D0(0, 2);
            func_8007F1D0(0, 4);
            func_8007F1D0(0, 6);
        }
        else if (extra)
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
    }
}
