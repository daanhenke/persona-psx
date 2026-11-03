#include <persona/casino/casino.h>

extern u_int D_800A96A4[11];
extern CasinoLayout *balance_layouts[][2] __asm__("D_800A2BC8+0x2554");
extern void CasinoSpritesSetOn();
extern void CasinoTexLayout(CasinoLayout *);

/* Draw four balance digits, suppressing leading zeroes except the units. */
void func_80090F58(u_char choice)
{
    u_int digits[4];
    u_int *digit;
    int seen;
    int i;
    int sprite;
    int sprite_base;
    int layout_offset;
    int on;
    int number;
    int first;
    u_short texture;


    digits[0] = D_800A96A4[choice] % 10000U / 1000U;
    digits[1] = D_800A96A4[choice] % 1000U / 100U;
    digits[2] = D_800A96A4[choice] % 100U / 10U;
    digits[3] = D_800A96A4[choice] % 10U;
    seen = 0;
    i = 0;
    sprite_base = choice * 4;
    layout_offset = choice * 8;
    digit = digits;
    sprite = 75;
    do
    {
        if (*digit != 0)
        {
            seen = 1;
        }
        if (i != 3)
        {
            if (seen)
            {
                first = sprite_base + sprite;
                number = 1;
                on = 1;
            }
            else
            {
                first = sprite_base + sprite;
                number = 1;
                on = 0;
            }
            CasinoSpritesSetOn(first, number, on);
        }
        texture = *digit;
        digit++;
        sprite++;
        (*(CasinoLayout **)((char *)balance_layouts+layout_offset))->tex_idx[i] = texture;
        i++;
        CasinoTexLayout((*(CasinoLayout **)((char *)balance_layouts+layout_offset)));
    } while (i < 4);
}
