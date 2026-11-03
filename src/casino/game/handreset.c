/* Persona 1 (JP) - reset blackjack hand counters and play state. */
#include <decomp/types.h>

extern u_char g_bj_card_obj;
extern u_char g_bj_natural[];
extern u_char g_bj_dealer_bj;
extern u_char D_800A8674;
extern u_char g_casino_pay_digit;
extern u_char g_casino_bet_done;
extern u_char g_casino_quit;
extern u_char D_800A8818[];
extern u_char g_bj_total[];
extern u_char g_bj_split_total[];
extern u_char g_bj_dealer_total[];
extern u_char D_800A8C7C;
extern u_char g_bj_count[];
extern u_char g_bj_dealer_count;
extern u_char g_bj_label_n[];

void func_8007DAFC(void)
{
    g_bj_card_obj = 0x18;
    g_bj_natural[0] = 0x1;
    g_bj_natural[1] = 0x1;
    g_bj_dealer_bj = 0x1;
    D_800A8674 = 0x0;
    g_casino_pay_digit = 0x0;
    g_casino_bet_done = 0x0;
    g_casino_quit = 0x0;
    D_800A8818[0] = 0x0;
    D_800A8818[1] = 0x0;
    g_bj_total[0] = 0x0;
    g_bj_total[1] = 0x0;
    g_bj_split_total[0] = 0x0;
    g_bj_split_total[1] = 0x0;
    g_bj_dealer_total[0] = 0x0;
    g_bj_dealer_total[1] = 0x0;
    D_800A8C7C = 0x12;
    g_bj_count[0] = 0x0;
    g_bj_count[1] = 0x0;
    g_bj_dealer_count = 0x0;
    g_bj_label_n[0] = 0x0;
    g_bj_label_n[1] = 0x0;
    g_bj_label_n[2] = 0x0;
}
