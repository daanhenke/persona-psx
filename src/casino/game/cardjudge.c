/* Persona 1 (JP) - advance blackjack state from flags and totals. */
#include <decomp/types.h>

extern u_char g_bj_natural;
extern u_char g_bj_dealer_bj;
extern s8 g_bj_play_step;
extern int g_casino_timer;
extern u_char player_total __asm__("g_bj_total+0x1");
extern u_char dealer_total __asm__("g_bj_dealer_total+0x1");

void func_8007A27C(void)
{
    u_char *natural = &g_bj_natural;
    if (*natural == 2) {
        g_bj_play_step = 23;
    } else if (g_bj_dealer_bj == 2) {
        if (*natural != 8) {
            *natural = 11;
        }
        g_bj_play_step = 22;
    } else if (*natural == 8) {
        if (g_bj_dealer_bj == 8) {
            g_bj_play_step = 25;
        } else {
            g_bj_play_step = 22;
        }
    } else if (player_total > dealer_total) {
        g_bj_play_step = 22;
        *natural = 11;
    } else if (player_total < dealer_total) {
        g_bj_play_step = 23;
        *natural = 12;
    } else if (player_total == dealer_total) {
        if (g_bj_dealer_bj == 8) {
            g_bj_play_step = 23;
            *natural = 12;
        } else {
            g_bj_play_step = 25;
            *natural = 13;
        }
    }
    g_casino_timer = -1;
}
