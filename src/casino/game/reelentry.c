/* Enter reel betting: clear input and digit state, then reveal the controls. */
#include <persona/casino/casino.h>
extern s8 g_casino_key_fire[], g_casino_key_hold[];
extern u_char D_800AF834[], D_800A85F8[];
extern short D_80099A20[];
extern CasinoPalAnim D_800999C0;
extern u_char reel_entry_palette_on __asm__("D_800999C0+0x10");
extern CasinoLayoutDef D_800A1764, D_800A17B8;
extern void CasinoCursorClear(CasinoCursor *);
extern void CasinoQueueCluts(short, short, int);
extern void CasinoSpritesSetOn(short, short, int);
extern void CasinoTween(CasinoLayout *, int, short, short, short, short);
extern void CasinoStartPalAnim(CasinoPalAnim *);

void func_80082C80(void)
{
    int i;
    u_char *p;
    u_char *q;
    if (g_casino_timer == 0) {
        CasinoCursorClear((CasinoCursor *)g_casino_key_fire);
        CasinoCursorClear((CasinoCursor *)g_casino_key_hold);
        CasinoQueueCluts(537, 21, GetClut(D_80099A20[4], D_80099A20[5]));
        CasinoSpritesSetOn(426, 13, 1);
        CasinoSpritesSetOn(439, 12, 1);
        CasinoTween(D_800A1764.l, 320, 0, 0, 0, 32);
        CasinoTween(D_800A17B8.l, -320, 0, 0, 0, 32);
        g_casino_max_bet = 0;
        D_800AF834[5] = 0;
        g_casino_pay_digit = 0;
        CASINO_LAMPS.b0 = 1;
        for (i = 7, p = g_casino_win_digits + 7; i >= 0; i--, p--)
            *p = 0;
        for (i = 12, q = D_800A85F8 + 38; i >= 0; i--, q--)
            *q = 0;
    }
    if (g_casino_timer >= 33) {
        if (reel_entry_palette_on == 0)
            CasinoStartPalAnim(&D_800999C0);
        CasinoSpritesSetOn(405, 14, 1);
        g_casino_timer = -1;
        g_casino_step = 38;
    }
}
