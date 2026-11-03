/* Accept grid play, return coins on exit, and refresh its status lamps. */
#include <persona/casino/casino.h>
extern u_char D_800A878C, D_800B0AF0;
extern s8 D_800AF73C[];
extern void *D_800A2BC8[];
extern void func_8008C8D0(void);
extern void CasinoStartAnim(CasinoObj *, short, short, short, short, long, long, long, long, long, long);
extern void CasinoPlaySeq(short *, u_long *, short);
extern void CasinoShowNumber(u_int, CasinoLayout *, u_char);

void func_8008A628(void)
{
    if (g_casino_bet_done == 1) {
        func_8008C8D0();
        g_casino_cursor.w = 3;
        g_casino_cursor.h = 3;
        g_casino_cursor.prev_x = 0;
        g_casino_cursor.x = 0;
        g_casino_cursor.prev_y = 0;
        g_casino_cursor.y = 0;
        g_casino_cursor.wrap_x = 1;
        g_casino_cursor.wrap_y = 1;
        CASINO_LAMPS.b0 = 0;
        CasinoStartAnim(D_800A2BC8[817], 16, 0, 0, 0, 0, -16, 0, 0, 0, 0);
        CasinoStartAnim(D_800A2BC8[851], 16, 0, 0, 0, 0, -16, 0, 0, 0, 0);
        g_casino_timer = -1;
        g_casino_bet_done = 0;
        g_casino_step = 30;
    }
    if (g_casino_quit == 1) {
        *g_casino_coin_item = (*g_casino_coin_item & 511) | (D_800A878C << 9);
        *g_casino_coin_item = (D_800A878C << 9) | 35;
        CasinoPlaySeq(&g_casino_seqs[3], (u_long *)0x12f00c, g_casino_main_vab);
        CASINO_LAMPS.b0 = 0;
        CASINO_LAMPS.b2 = 0;
        CasinoFade(0, 333, 0, 0, 0, 32);
        g_casino_timer = -1;
        g_casino_game = 31;
        g_casino_step = 254;
        g_casino_quit = 0;
    }
    if (CASINO_LAMPS.b0 != CASINO_LAMPS.b1) {
        CasinoStartAnim(D_800A2BC8[783], 16, 2048, 0, 0, 0, 0, 0, 0, 0, 0);
        CASINO_LAMPS.b1 = CASINO_LAMPS.b0;
    }
    if (CASINO_LAMPS.b2 != CASINO_LAMPS.b3) {
        CasinoStartAnim(D_800A2BC8[817], 16, 2048, 0, 0, 0, 0, 0, 0, 0, 0);
        CASINO_LAMPS.b3 = CASINO_LAMPS.b2;
    }
    if (D_800A878C != D_800B0AF0) {
        CasinoShowNumber(D_800A878C, D_800A2BC8[101], 2);
        D_800B0AF0 = D_800A878C;
    }
    if (g_casino_pad_trig & 2048) D_800AF73C[5] = !D_800AF73C[5];
}
