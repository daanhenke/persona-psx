#include <persona/casino/casino.h>
#include <libsnd.h>

extern void CasinoPlaySeq(short *, unsigned int, short);

void func_800856A0(void)
{
    if (g_casino_pad_trig & 16)
    {
        if (g_casino_money)
        {
            g_casino_max_bet = 1;
            CASINO_LAMPS.b0 = 0;
            CASINO_LAMPS.b4 = 0;
        }
        else
            CasinoPlaySeq(&g_casino_seqs[0], 0x125F2C, g_casino_main_vab);
    }
    else if (g_casino_pad_trig & 32)
    {
        if (g_casino_money)
        {
            SsSeqStop(g_casino_seqs[12]);
            SsSeqPlay(g_casino_seqs[12], 1, 1);
            g_casino_bet++;
            g_casino_money--;
        }
        else
            CasinoPlaySeq(&g_casino_seqs[0], 0x125F2C, g_casino_main_vab);
    }
    else if (g_casino_pad_trig & 64)
    {
        if (g_casino_bet)
        {
            SsSeqStop(g_casino_seqs[5]);
            SsSeqPlay(g_casino_seqs[5], 1, 1);
            g_casino_bet = 0;
            g_casino_money += g_casino_bet_shown;
        }
    }
    else if (g_casino_pad_trig & 8 || g_casino_pad_trig & 2)
    {
        if (g_casino_bet)
            g_casino_bet_done = 1;
        else
            CasinoPlaySeq(&g_casino_seqs[0], 0x125F2C, g_casino_main_vab);
    }
    else if (g_casino_pad_trig & 4 || g_casino_pad_trig & 1)
    {
        if (!g_casino_bet_done)
            g_casino_quit = 1;
    }
}
