#include <persona/casino/casino.h>

extern CasinoXform g_casino_xforms[];
extern unsigned char func_80082454(unsigned char);
extern void func_800824CC(int, unsigned char, int, int);
extern void func_800825A4(unsigned char, int);
extern void CasinoSetXform(CasinoXform *);
extern void CasinoDrawObj(CasinoObj *);

void D_80080178(unsigned char *card, unsigned char slot, unsigned char forward)
{
    unsigned char rank;
    int x;

    if (forward) {
        (*card)++;
        if (*card >= 52)
            *card = 0;
    } else {
        (*card)--;
        if ((signed char)*card < 0)
            *card = 51;
    }
    rank = func_80082454(*card);
    x = slot * 20 + 140;
    func_800824CC(slot, *card, rank, x);
    func_800825A4(*card, x);
    CasinoSetXform(&g_casino_xforms[slot]);
    CasinoDrawObj(&g_casino_objs[slot]);
}
