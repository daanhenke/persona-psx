/* Persona 1 (JP) - the party's money.
 *
 *   ADV @ 0x800AAFD4
 *
 * The counter lives in the save-game work area, eight bytes before g_items,
 * and this is the only thing that adds to it. The cap is 999,999,999 and the
 * comparison against it is unsigned, so a total that would overflow past the
 * cap sticks there rather than wrapping.
 */
#include <types.h>

#define g_money (*(u_int *)0x801F2674)

#define MONEY_MAX 999999999

void MoneyAdd(u_int amount)
{
    g_money = g_money + amount;
    if (g_money > MONEY_MAX) {
        g_money = MONEY_MAX;
    }
}
