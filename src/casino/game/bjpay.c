/* Blackjack hand payout and confirmation state. */
extern struct
{
    char pad[8];
    short x, y;
    char pad2[4];
    unsigned char active;
} D_80099990;
extern unsigned char g_bj_result_palette_on, g_bj_natural[], D_800A8678[], D_800A8818[],
    g_casino_bet, g_bj_play_step, g_casino_pay_digit;
extern short D_80099DC0[], g_casino_seqs[];
extern int g_casino_timer, g_casino_win, D_80099DD0, g_casino_frame;
extern unsigned int g_bj_jackpot, g_casino_money, g_casino_pad_trig;
extern void CasinoStartPalAnim(void *), func_8007F7E8(int), func_8007F668(int),
    func_8007FBD8(unsigned char), func_8007F5F8(void *), CasinoSplitDigits(int),
    CasinoQueueCluts(int, int, unsigned short), func_80080094(unsigned char, int),
    SsSeqSetVol(short, short, short), SsSeqPlay(short, char, short), SsSeqStop(short),
    CasinoPayStep(int *);
extern unsigned short GetClut(short, short);
extern short SsIsEos(short, short);
void func_8007C514(unsigned char hand, unsigned char next)
{
    if (!g_bj_result_palette_on)
        CasinoStartPalAnim(&D_80099990);
    if (!g_casino_timer)
    {
        if (D_800A8678[hand] == 2)
        {
            if (!hand)
                func_8007F7E8(1);
            else
                func_8007F668(1);
        }
        if (g_bj_natural[hand] == 1)
        {
            g_bj_natural[hand] = 11;
            func_8007FBD8(hand);
        }
        func_8007F5F8(&g_bj_natural[hand]);
        if (!g_bj_natural[hand])
        {
            g_casino_win = (int)(g_bj_jackpot / 100) * (D_800A8818[hand] + 1);
            g_bj_jackpot = 300000;
        }
        else
            g_casino_win = g_casino_bet * D_80099DC0[g_bj_natural[hand]];
        CasinoSplitDigits(g_casino_win);
        CasinoQueueCluts(g_bj_natural[hand] + 748, 1, GetClut(D_80099990.x, D_80099990.y));
        CasinoQueueCluts(g_bj_natural[hand] + 755, 1, GetClut(D_80099990.x, D_80099990.y));
        func_80080094(g_bj_natural[hand], 1);
        SsSeqSetVol(g_casino_seqs[15], 64, 64);
    }
    if (!SsIsEos(g_casino_seqs[1], 0) && g_casino_timer >= 0)
    {
        SsSeqSetVol(g_casino_seqs[15], 127, 127);
        if (g_casino_pad_trig & 32 || g_casino_pad_trig & 128 || g_casino_pad_trig & 16 ||
            g_casino_pad_trig & 64 || g_casino_pad_trig & 8 || g_casino_pad_trig & 2 ||
            g_casino_pad_trig & 8192 || g_casino_pad_trig & 32768 || g_casino_pad_trig & 4096 ||
            g_casino_pad_trig & 16384 || g_casino_pad_trig & 4 || g_casino_pad_trig & 1)
        {
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            D_80099DD0 = 1;
        }
    }
    if (D_80099DD0)
    {
        if (!(g_casino_frame & 1))
        {
            SsSeqStop(g_casino_seqs[6]);
            SsSeqPlay(g_casino_seqs[6], 1, 1);
            CasinoPayStep(&g_casino_win);
        }
        if (g_casino_money >= 99999999)
        {
            g_casino_win = 0;
            g_casino_money = 99999999;
        }
        if (!g_casino_win)
        {
            SsSeqStop(g_casino_seqs[6]);
            CasinoQueueCluts(748, 7, 0x7d64);
            CasinoQueueCluts(755, 7, 0x7d64);
            D_80099DD0 = 0;
            g_casino_pay_digit = 0;
            g_casino_timer = -1;
            g_bj_play_step = next;
        }
    }
}
