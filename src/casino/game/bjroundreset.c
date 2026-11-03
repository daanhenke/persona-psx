/* Reset the blackjack cursor, bet bookkeeping, and lamp state. */
#include <persona/casino/casino.h>

extern u_char g_bj_reshuffle, D_800B0AF8[], g_bj_shoe_pos, g_bj_insurance;
extern u_int g_casino_money_shown;
extern int g_bj_jackpot;
extern u_char hand_reset_flag __asm__("D_800AAEA8+4");

void func_8007DA00(void)
{
    g_bj_reshuffle = 1;
    D_800B0AF8[1] = 0;
    g_casino_cursor.prev_x = 0;
    g_casino_cursor.x = 0;
    g_casino_cursor.prev_y = 0;
    g_casino_cursor.y = 0;
    g_casino_cursor.w = 12;
    g_casino_cursor.h = 12;
    g_casino_cursor.wrap_x = 1;
    g_casino_cursor.wrap_y = 1;
    g_casino_money_shown = g_casino_money;
    g_bj_shoe_pos = 0;
    g_casino_max_bet = 0;
    g_casino_bet = 0;
    g_casino_bet_shown = 0;
    g_bj_insurance = 0;
    g_casino_jackpot_add = 0;
    g_casino_jackpot_hit = 0;
    hand_reset_flag = 0;
    g_casino_jackpot_shown = g_bj_jackpot;
    CASINO_LAMPS.b5 = 0;
    CASINO_LAMPS.b4 = 0;
    CASINO_LAMPS.b3 = 0;
    CASINO_LAMPS.b2 = 0;
    CASINO_LAMPS.b1 = 0;
    CASINO_LAMPS.b0 = 0;
}
