/* Persona JP1: display the reel payout, then clear its win highlights. */
#include <decomp/types.h>

extern u_char D_800AF9E4[], g_casino_bet, D_800A8408[];
/* Result kind and rank occupy adjacent bytes 39 and 40. */
extern signed char D_800A85F8[];
extern short D_8009E8E8[][6], D_80099A38[];
extern short g_casino_seqs[], g_casino_main_vab;
extern short payout_ticks __asm__("D_800AF834+218");
extern int g_casino_win;
extern void CasinoPlaySeq(short *, unsigned int, short);
extern void CasinoSpritesSetOn(int, int, int);
extern void CasinoQueueCluts(int, int, unsigned short);
extern void func_80084444(int), func_800844D4(int), CasinoSplitDigits(int);
extern unsigned short GetClut(short, short);
extern short SsIsEos(short, short);

void func_800840C8(void)
{
    int i;

    switch (D_800AF9E4[3]) {
    case 2:
        switch (D_800A85F8[39]) {
        case 5:
            if (D_800A85F8[40] == 6) {
                CasinoPlaySeq(g_casino_seqs, 0x126374, g_casino_main_vab);
            } else {
                CasinoPlaySeq(g_casino_seqs, 0x125F94, g_casino_main_vab);
            }
            break;
        case 4:
            CasinoPlaySeq(g_casino_seqs, 0x1260C4, g_casino_main_vab);
            break;
        case 3:
            CasinoPlaySeq(g_casino_seqs, 0x1261A4, g_casino_main_vab);
            break;
        }
        g_casino_win += D_8009E8E8[D_800A85F8[40]][D_800A85F8[39]] * g_casino_bet;
        CasinoSpritesSetOn(537, 21, 0);
        for (i = 0; i < 5; i++) {
            if (D_800A8408[537 + i] == 1) {
                CasinoSpritesSetOn(537 + i, 1, 1);
            }
        }
        CasinoQueueCluts(D_800A85F8[40] * 24 + D_800A85F8[39] * 4 + 208,
                         4, GetClut(D_80099A38[4], D_80099A38[5]));
        CasinoQueueCluts((D_800A85F8[39] * 8 + 161) + D_800A85F8[40], 1, 0x7DA5);
        func_80084444(1);
        func_800844D4(1);
        payout_ticks = 0;
        D_800AF9E4[3] = 5;
        break;
    case 5:
        if (!SsIsEos(g_casino_seqs[0], 0)) {
            D_800AF9E4[3] = 7;
        }
        payout_ticks++;
        break;
    case 7:
        func_800844D4(0);
        func_80084444(0);
        CasinoQueueCluts(D_800A85F8[40] * 24 + D_800A85F8[39] * 4 + 208, 4, 0x7C25);
        CasinoQueueCluts((D_800A85F8[39] * 8 + 161) + D_800A85F8[40], 1, 0x7D65);
        for (i = 0; i < 5; i++) {
            if (D_800A8408[537 + i] == 1) {
                CasinoSpritesSetOn(537 + i, 1, 0);
            }
        }
        CasinoSplitDigits(g_casino_win);
        D_800AF9E4[3] = 8;
        break;
    }
}
