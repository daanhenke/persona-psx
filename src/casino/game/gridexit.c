#include <persona/casino/casino.h>

extern int D_800A96A4[11];
extern u_char D_800AAE9C;
extern CasinoLayout *money_layout __asm__("D_800A2BC8+0x20F0");
extern CasinoObj *exit_object0 __asm__("D_800A2BC8+0x39F0");
extern CasinoObj *exit_object1 __asm__("D_800A2BC8+0x3A04");
extern void CasinoPlaySeq(short *, u_int, short);
extern void CasinoQueueCluts();
extern void CasinoShowNumber();
extern void CasinoStartAnim();
extern void func_80090F58(int);

/* Refund all eleven balances, refresh their displays, and leave the grid. */
void func_80090A50(void)
{
    int *palette_balance;
    int index;
    int payout_index;
    int amount;
    int money;
    int i;
    int first;
    int count;

    g_casino_quit = 0;
    CASINO_LAMPS.b0 = 0;
    CASINO_LAMPS.b2 = 0;
    CASINO_LAMPS.b4 = 0;
    CasinoPlaySeq(g_casino_seqs, 0x13B220, g_casino_main_vab);
    CasinoFade(0, 416, 0, 0, 0, 32);
    payout_index = 0;
    do
    {
        amount = D_800A96A4[payout_index];
        index = payout_index & 255;
        D_800A96A4[payout_index] = 0;

        money = g_casino_money;
        money = money + amount;
        g_casino_money = money;
        func_80090F58(index);
        payout_index++;
    } while (payout_index < 11);
    count = 0;
    i = 0;
    first = 64;
    palette_balance = D_800A96A4;
    do
    {
        if (*palette_balance != 0)
        {
            CasinoQueueCluts(first, 1, 0x7CE5);
            count++;
        }
        else
        {
            CasinoQueueCluts(first, 1, 0x7CA5);
        }
        first++;
        i++;
        palette_balance++;
    } while (i < 11);
    if (count != 0)
    {
        D_800AAE9C = 1;
    }
    else
    {
        D_800AAE9C = 0;
    }
    CasinoShowNumber(g_casino_money, money_layout, 8);
    CasinoStartAnim(exit_object0, 64, 0, 0, 0, 0, 0, 0, -4096, -4096, -4096);
    CasinoStartAnim(exit_object1, 64, 0, 0, 0, 0, 0, 0, -4096, -4096, -4096);
    g_casino_timer = -1;
    g_casino_game = 31;
    g_casino_step = 0xFE;
}
