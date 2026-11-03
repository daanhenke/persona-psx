#include <persona/casino/casino.h>
extern int puts(const char *);
extern void func_8007DA00(void);
u_char g_bj_reshuffle, D_800B0AF8[8], g_bj_shoe_pos, g_bj_insurance, D_800AAEA8[8];
u_int g_casino_money, g_casino_money_shown, g_casino_lamps;
int g_bj_jackpot;
CasinoCursor g_casino_cursor;
u_char g_casino_max_bet, g_casino_bet, g_casino_bet_shown;
u_char g_casino_jackpot_add, g_casino_jackpot_hit;
int g_casino_jackpot_shown;
int main(void) {
    unsigned bits;int i;
    for(bits=0;bits<256;bits++) {
        g_casino_lamps=0xA5010000U|bits;
        g_casino_money=0xFFFFFFFFU-bits;g_casino_money_shown=0;
        g_bj_jackpot=-1-(int)bits;g_casino_jackpot_shown=0;
        g_bj_reshuffle=29;g_bj_shoe_pos=29;g_bj_insurance=29;
        g_casino_max_bet=g_casino_bet=g_casino_bet_shown=29;
        g_casino_jackpot_add=g_casino_jackpot_hit=29;
        for(i=0;i<8;i++) D_800B0AF8[i]=D_800AAEA8[i]=29;
        for(i=0;i<(int)sizeof g_casino_cursor;i++) ((u_char *)&g_casino_cursor)[i]=29;
        func_8007DA00();
        if(g_casino_lamps!=((0xA5010000U|bits)&~63U) ||
           g_casino_money_shown!=0xFFFFFFFFU-bits || g_casino_money!=0xFFFFFFFFU-bits ||
           g_casino_jackpot_shown!=-1-(int)bits || g_bj_jackpot!=-1-(int)bits) return 1;
        if(g_bj_reshuffle!=1 || g_bj_shoe_pos || g_bj_insurance ||
           g_casino_max_bet || g_casino_bet || g_casino_bet_shown ||
           g_casino_jackpot_add || g_casino_jackpot_hit) return 2;
        for(i=0;i<8;i++) if(D_800B0AF8[i]!=(i==1?0:29) || D_800AAEA8[i]!=(i==4?0:29)) return 3;
        for(i=0;i<(int)sizeof g_casino_cursor;i++) {
            int expected=i<4?0:i<6?12:i<8?1:29;
            if(((u_char *)&g_casino_cursor)[i]!=expected) return 4;
        }
    }
    puts("256 native blackjack round-reset fixtures passed");return 0;
}
