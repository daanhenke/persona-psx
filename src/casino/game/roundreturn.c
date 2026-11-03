#include <persona/casino/casino.h>

extern CasinoLayout *left_layout __asm__("D_800A2BC8+0x2C68");
extern CasinoLayout *middle_layout __asm__("D_800A2BC8+0x2CC4");
extern CasinoLayout *right_layout __asm__("D_800A2BC8+0x2D20");
extern CasinoLayout *frame_layout __asm__("D_800A2BC8+0x2ADC");
extern s8 g_casino_key_fire[4];
extern s8 g_casino_key_hold[4];
extern void CasinoCursorClear(CasinoCursor *);
extern void CasinoTween();
extern void CasinoSpritesSetOn();

void func_8008D4C4(void)
{
    int i;
    u_char *digit;

    if (g_casino_timer == 0)
    {
        CasinoTween(left_layout, 320, 0, 0, 0, 32);
        CasinoTween(middle_layout, -320, 0, 0, 0, 32);
        CasinoTween(right_layout, 320, 0, 0, 0, 32);
        i = 7;
        digit = &g_casino_win_digits[7];
        g_casino_pay_digit = 0;
        do
        {
            *digit = 0;
            i--;
            digit--;
        } while (i >= 0);
        CasinoCursorClear((CasinoCursor *)g_casino_key_fire);
        CasinoCursorClear((CasinoCursor *)g_casino_key_hold);
    }
    if (g_casino_timer == 24)
    {
        CasinoTween(frame_layout, 0, -40, 0, 80, 8);
    }
    if (g_casino_timer >= 32)
    {
        CasinoSpritesSetOn(218, 12, 1);
        g_casino_timer = -1;
        g_casino_step = 0x26;
        CASINO_LAMPS.b0 = 1;
    }
}
