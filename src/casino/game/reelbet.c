/* Reel-game betting, display updates, acceptance, and refunds. */
#include <persona/casino/casino.h>

extern CasinoPalAnim D_800999C0;
extern u_char reel_bet_palette_on __asm__("D_800999C0+0x10");
extern CasinoLayoutDef D_800A10D8;
extern void CasinoStartPalAnim(CasinoPalAnim *);
extern void func_800856A0(void), func_80085874(void), func_80087F48(u_char);
extern void func_80085920(u_char, short, short);
extern void SsSeqStop(short), SsSeqPlay(short, char, short);
extern void CasinoQueueCluts(short, short, int);
extern void CasinoScroll(CasinoLayout *, short, short, short, short, short);
extern void CasinoBuildLayouts(CasinoLayoutDef *, int);
extern void CasinoPlaySeq(short *, u_long *, short);

void func_80082E1C(void)
{
    if (!reel_bet_palette_on)
        CasinoStartPalAnim(&D_800999C0);
    if (g_casino_timer > 0 && !g_casino_max_bet)
        func_800856A0();
    if (g_casino_max_bet && !(g_casino_timer & 1)) {
        g_casino_bet++;
        g_casino_money--;
        if (g_casino_bet != 8 || g_casino_money) {
            SsSeqStop(g_casino_seqs[12]);
            SsSeqPlay(g_casino_seqs[12], 1, 1);
        }
    }
    if (g_casino_bet != g_casino_bet_shown) {
        if (g_casino_bet) {
            CasinoQueueCluts(493, 5, GetClut(848, g_casino_bet + 495));
            CasinoScroll(D_800A10D8.l,
                         g_casino_bet * 16 - g_casino_bet_shown * 16,
                         0, 0, 0, 1);
            func_80085920(1, g_casino_bet, g_casino_bet_shown);
        } else {
            CasinoQueueCluts(493, 5, 0x7C27);
            if (g_casino_bet_shown != 255)
                CasinoScroll(D_800A10D8.l,
                             g_casino_bet * 16 - g_casino_bet_shown * 16,
                             0, 0, 0, 1);
            func_80085920(0, g_casino_bet, g_casino_bet_shown);
        }
        func_80085874();
        g_casino_bet_shown = g_casino_bet;
    }
    if (g_casino_bet == 8 || (g_casino_bet && !g_casino_money))
        g_casino_bet_done = 1;
    if (g_casino_bet_done == 1) {
        SsSeqStop(g_casino_seqs[13]);
        SsSeqPlay(g_casino_seqs[13], 1, 1);
        func_80087F48(1);
        CASINO_LAMPS.b0 = 0;
        CASINO_LAMPS.b4 = 0;
        if (CASINO_LAMPS.b3)
            CASINO_LAMPS.b2 = 0;
        g_casino_step = 112;
        g_casino_timer = -1;
        g_casino_bet_done = 0;
    }
    if (g_casino_quit == 1) {
        CasinoPlaySeq(g_casino_seqs, (u_long *)0x125F54, g_casino_main_vab);
        if (g_casino_bet) {
            SsSeqStop(g_casino_seqs[5]);
            SsSeqPlay(g_casino_seqs[5], 1, 1);
            g_casino_money = g_casino_bet + g_casino_money;
            g_casino_bet = 0;
            CasinoQueueCluts(493, 5, 0x7C27);
            CasinoBuildLayouts(&D_800A10D8, 0);
            func_80085920(0, g_casino_bet, g_casino_bet_shown);
            func_80085874();
        }
        CASINO_LAMPS.b0 = 0;
        CASINO_LAMPS.b2 = 0;
        CASINO_LAMPS.b4 = 0;
        CasinoFade(0, 558, 0, 0, 0, 32);
        g_casino_timer = -1;
        g_casino_game = 31;
    }
}
