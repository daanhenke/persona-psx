#ifndef PERSONA_CASINO_REWARD_H
#define PERSONA_CASINO_REWARD_H
#include <persona/casino/casino.h>

/* Dice sums 2–12 select reward values; preserve the intervening work area. */
typedef struct {
    u_int values[13];
    u_char reserved[84];
    u_char dice[3];
} CasinoRewardData;
extern CasinoRewardData g_casino_reward_data __asm__("D_800A9724-136");
#endif
