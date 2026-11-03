/* Card betting input, preserving button priority and refund behavior. */
extern unsigned int g_casino_pad_trig, g_casino_money;
extern unsigned char g_casino_bet, g_casino_max_bet, g_casino_bet_done, g_casino_quit;
extern short g_casino_seqs[], g_casino_main_vab;
extern void SsSeqStop(short), SsSeqPlay(short, char, short),
    CasinoPlaySeq(short *, unsigned int, short);
void func_8007E30C(void)
{
    if (g_casino_pad_trig & 16)
    {
        if (g_casino_money)
        {
            if (g_casino_bet < 10)
                g_casino_max_bet = 1;
            else
                g_casino_bet_done = 1;
        }
        else
            CasinoPlaySeq(g_casino_seqs, 0x138858, g_casino_main_vab);
    }
    else if (g_casino_pad_trig & 32)
    {
        if (g_casino_bet < 10)
        {
            if (g_casino_money)
            {
                SsSeqStop(g_casino_seqs[6]);
                SsSeqPlay(g_casino_seqs[6], 1, 1);
                g_casino_bet++;
                g_casino_money--;
            }
            else
                CasinoPlaySeq(g_casino_seqs, 0x138858, g_casino_main_vab);
        }
        else
            g_casino_bet_done = 1;
    }
    else if ((g_casino_pad_trig & 64) && !g_casino_max_bet)
    {
        if (g_casino_bet)
        {
            SsSeqStop(g_casino_seqs[5]);
            SsSeqPlay(g_casino_seqs[5], 1, 1);
            g_casino_money += g_casino_bet;
            g_casino_bet = 0;
        }
    }
    else if ((g_casino_pad_trig & 8 || g_casino_pad_trig & 2) && !g_casino_max_bet)
    {
        if (g_casino_bet)
        {
            g_casino_bet_done = 1;
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
        }
        else
            CasinoPlaySeq(g_casino_seqs, 0x138858, g_casino_main_vab);
    }
    else if (g_casino_pad_trig & 4 || g_casino_pad_trig & 1)
    {
        if (!g_casino_bet_done)
            g_casino_quit = 1;
    }
}
