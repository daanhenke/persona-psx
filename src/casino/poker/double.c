/* Persona 1 (JP) - CASINO's video poker: leaving the double-up, and the
 * pieces its three games call.
 *   0x800775CC CasinoPokerDoubleExit
 *   0x80077F38 CasinoPokerDoubleMusic
 *   0x80078014 CasinoPokerDoubleMusicEnd
 *   0x80078094 CasinoPokerDoubleBlink
 *   0x80078154 CasinoPokerDummy2
 *   0x8007815C CasinoPokerDummy3
 *   0x80078164 CasinoPokerHiLoKeys
 *   0x800781F4 CasinoPokerDummy4
 *   0x800781FC CasinoPokerColorKeys
 *
 * The way out of the double-up undoes its intro: the table's sprites come
 * back on, the double-up's panels slide off left and right, the table's
 * pieces slide back down and the two readouts in from the sides, and the
 * round starts over at POKER_ENTER.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>
#include <persona/casino/poker.h>

extern void CasinoTween();        /* (CasinoLayout *l, short dx, short dy, short dw, short dh, short frames) */
extern void CasinoSpritesSetOn(); /* (short first, short n, int on) */
extern void CasinoShowLayout(CasinoLayout *l, u_char mode);
extern void CasinoSetXform(CasinoXform *x);
extern void CasinoDrawObj(CasinoObj *o);
extern void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                            long sy, long sz);
extern void CasinoPlaySeq(short *h, u_long *seq, short vab);
extern void CasinoCloseSeq(short *h);
extern void CasinoPokerShowBet(void);

/* The table's pieces the double-up pushes off the bottom. */
extern CasinoLayout D_80094940;
extern CasinoLayout D_8009488C;
extern CasinoLayout D_80094A08;
extern CasinoLayout D_80094A48;
extern CasinoLayout D_80094B48;
extern CasinoLayout D_80094B60;
extern CasinoLayout D_80094C20;
extern CasinoLayout D_80094C38;
extern CasinoLayout D_80094C50;
extern CasinoLayout D_80094C68;
extern CasinoLayout D_80094C80;
extern CasinoLayout D_800951CC;
extern CasinoLayout D_8009523C;
extern CasinoLayout D_800952AC;
extern CasinoLayout D_8009531C;
extern CasinoLayout D_80096660; /* the high-low game's two calls */
extern CasinoLayout D_800966F8;
extern CasinoLayout D_80096770; /* red or black's */
extern CasinoLayout D_80096808;

extern u_char g_poker_hilo_guess;
extern u_char g_poker_color_guess;

void CasinoPokerDoubleExit(void)
{
    CasinoLayout *table[11] = { &D_80094940, &D_8009488C, &D_80094A08, &D_80094A48, &D_80094B48, &D_80094B60,
                                &D_80094C20, &D_80094C38, &D_80094C50, &D_80094C68, &D_80094C80 };
    CasinoLayout *rows[6] = { &D_80094D9C,     &D_80094E28[0], &D_80094E28[1],
                              &D_80094E28[2], &D_80094E28[3], &D_80094E28[4] };
    CasinoLayout *jackpot[14] = { &g_poker_jackpot_digits[0],
                                  &g_poker_jackpot_digits[1],
                                  &g_poker_jackpot_digits[2],
                                  &g_poker_jackpot_digits[3],
                                  &g_poker_jackpot_digits[4],
                                  &g_poker_jackpot_digits[5],
                                  &g_poker_jackpot_digits[6],
                                  &g_poker_jackpot_digits[7],
                                  &g_poker_jackpot_digits[8],
                                  &D_80095164,
                                  &D_800951CC,
                                  &D_8009520C,
                                  &D_800952AC,
                                  &D_800952EC };
    CasinoLayout *money[13] = { &g_poker_money_digits[0],
                                &g_poker_money_digits[1],
                                &g_poker_money_digits[2],
                                &g_poker_money_digits[3],
                                &g_poker_money_digits[4],
                                &g_poker_money_digits[5],
                                &g_poker_money_digits[6],
                                &g_poker_money_digits[7],
                                &D_8009519C,
                                &D_8009523C,
                                &D_8009527C,
                                &D_8009531C,
                                &D_8009535C };
    CasinoLayout *left[19] = { &D_80095CF8,    &D_80095D30,    &D_80095E14,    &D_80095E94[0], &D_80095E94[1],
                               &D_80095E94[2], &D_80095E94[3], &D_80095E94[4], &D_80095E94[5], &D_80095E94[6],
                               &D_80095E94[7], &D_800960E4[0], &D_800960E4[1], &D_800960E4[2], &D_800960E4[3],
                               &D_800960E4[4], &D_800960E4[5], &D_800960E4[6], &D_800960E4[7] };
    CasinoLayout *right[9] = { &D_80095D68,    &D_80095FBC[0], &D_80095FBC[1], &D_80095FBC[2], &D_80095FBC[3],
                               &D_80095FBC[4], &D_80095FBC[5], &D_80095FBC[6], &D_80095FBC[7] };
    int i;

    if (g_casino_timer == 0) {
        CasinoSpritesSetOn(0x208, 9, 1);
        CasinoSpritesSetOn(0x211, 8, 1);
        CasinoSpritesSetOn(0x219, 4, 1);
        CasinoSpritesSetOn(0x21D, 4, 1);
        CasinoSpritesSetOn(0x221, 4, 1);
        CasinoSpritesSetOn(0x225, 4, 1);
        CasinoSpritesSetOn(0x22D, 1, 1);
        CasinoSpritesSetOn(0x22E, 1, 1);
        CasinoSpritesSetOn(0x22F, 1, 1);
        CasinoSpritesSetOn(0x230, 1, 1);
        CasinoSpritesSetOn(0x231, 1, 1);
        CasinoSpritesSetOn(0x232, 1, 1);
        CasinoSpritesSetOn(0x14F, 5, 1);
        CasinoSpritesSetOn(0x154, 0x24, 1);
        CasinoSpritesSetOn(0x178, 0x24, 1);
        CasinoSpritesSetOn(0x19C, 0x24, 1);
        CasinoSpritesSetOn(0x1C0, 0x24, 1);
        CasinoSpritesSetOn(0x1E4, 0x24, 1);
        CasinoSpritesSetOn(0xF9, 9, 1);
        CasinoSpritesSetOn(0xF5, 4, 1);
        CasinoSpritesSetOn(0x102, 0xA, 1);
        CasinoSpritesSetOn(0x10C, 4, 1);
        CasinoSpritesSetOn(0x119, 9, 1);
        CasinoSpritesSetOn(0x110, 4, 1);
        CasinoSpritesSetOn(0x11D, 0xA, 1);
        CasinoSpritesSetOn(0x127, 0xA, 1);
        CasinoSpritesSetOn(0x131, 0xA, 1);
        CasinoSpritesSetOn(0x13B, 0xA, 1);
        CasinoSpritesSetOn(0x145, 0xA, 1);
    }
    if (g_casino_timer == 0) {
        CasinoPlaySeq(&g_casino_seqs[3], (u_long *)0x139E18, g_casino_main_vab);
        for (i = 0; i < 19; i++) {
            CasinoTween(left[i], -0x80, 0, 0, 0, 0x20);
        }
        for (i = 0; i < 9; i++) {
            CasinoTween(right[i], 0x80, 0, 0, 0, 0x20);
        }
    }
    if (g_casino_timer == 0x20) {
        CasinoSpritesSetOn(0x2D9, 6, 0);
        CasinoSpritesSetOn(0x2DF, 8, 0);
        CasinoSpritesSetOn(0x2E7, 6, 0);
        CasinoSpritesSetOn(0x2ED, 8, 0);
        CasinoSpritesSetOn(0x298, 1, 0);
        CasinoSpritesSetOn(0x2CE, 0xB, 0);
        CasinoSpritesSetOn(0x2C7, 7, 0);
        CasinoSpritesSetOn(0x277, 8, 0);
        CasinoSpritesSetOn(0x27F, 0xB, 0);
        CasinoSpritesSetOn(0x28A, 0xB, 0);
        CasinoSpritesSetOn(0x295, 1, 0);
        CasinoSpritesSetOn(0x296, 1, 0);
        CasinoSpritesSetOn(0x297, 1, 0);
        CasinoSpritesSetOn(0x299, 1, 0);
        CasinoSpritesSetOn(0x29A, 8, 0);
        CasinoSpritesSetOn(0x2A2, 8, 0);
        CasinoSpritesSetOn(0x2AA, 8, 0);
        g_casino_bet = 0;
        CasinoPokerShowBet();
        g_casino_bet_shown = g_casino_bet;
        for (i = 0; i < 11; i++) {
            CasinoTween(table[i], 0, 0x80, 0, 0, 0x20);
        }
    }
    if (g_casino_timer == 0x40) {
        for (i = 0; i < 6; i++) {
            CasinoTween(rows[i], 0, -4, 0, 8, 4);
        }
    }
    if (g_casino_timer == 0x40) {
        for (i = 0; i < 14; i++) {
            CasinoTween(jackpot[i], 0xA0, 0, 0, 0, 0x20);
        }
        for (i = 0; i < 13; i++) {
            CasinoTween(money[i], -0xA0, 0, 0, 0, 0x20);
        }
        CasinoStartAnim(D_800953E0, 0x20, 0, 0, 0, 0xA0, 0, 0, 0, 0, 0);
        CasinoStartAnim(D_80095470, 0x20, 0, 0, 0, -0xA0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x60) {
        CasinoSpritesSetOn(0x233, 0x31, 1);
        CasinoTween(&D_800954C4, 0x10, -8, -0x20, 8, 8);
        D_80095770->xform->rot.vx = 0x800;
        D_80095770->xform->rot.vy = 0;
        D_80095770->xform->rot.vz = 0;
        D_80095770->xform->trans.vx = 0;
        D_80095770->xform->trans.vy = 0x48;
        D_80095770->xform->trans.vz = 0x200;
        D_80095770->xform->scale.vx = 0x1000;
        D_80095770->xform->scale.vy = 0x1000;
        D_80095770->xform->scale.vz = 0x1000;
        CasinoSetXform(D_80095770->xform);
        CasinoDrawObj(D_80095770);
        D_800955C4->xform->rot.vx = -0x800;
        D_800955C4->xform->rot.vy = 0;
        D_800955C4->xform->rot.vz = 0;
        D_800955C4->xform->trans.vx = -8;
        D_800955C4->xform->trans.vy = 0x28;
        D_800955C4->xform->trans.vz = 0x200;
        D_800955C4->xform->scale.vx = 0x1000;
        D_800955C4->xform->scale.vy = 0x1000;
        D_800955C4->xform->scale.vz = 0x1000;
        CasinoSetXform(D_800955C4->xform);
        CasinoDrawObj(D_800955C4);
        CasinoShowLayout(&D_800957E4[0], 1);
        CasinoShowLayout(&D_800957E4[1], 1);
        CasinoShowLayout(&D_800957E4[2], 1);
        CasinoShowLayout(&D_800957E4[3], 1);
        CasinoShowLayout(&D_800957E4[4], 1);
        g_casino_timer = -1;
        g_casino_step = POKER_ENTER;
    }
    if (g_casino_timer > 0x76) {
        g_casino_timer = -1;
        g_casino_step = POKER_ENTER;
    }
}

/* The win jingle, by what the double-up would pay. */
void CasinoPokerDoubleMusic(void)
{
    if (g_poker_double_pay != 0 && g_poker_double_pay < 100) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x13A418, g_casino_main_vab);
    } else if (g_poker_double_pay >= 100 && g_poker_double_pay < 1000) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x13A338, g_casino_main_vab);
    } else if (g_poker_double_pay >= 1000 && g_poker_double_pay < 1000000) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x13A208, g_casino_main_vab);
    } else if (g_poker_double_pay >= 1000000) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139FB8, g_casino_main_vab);
    }
}

void CasinoPokerDoubleMusicEnd(void)
{
    if (g_poker_double_pay != 0 && g_poker_double_pay < 100) {
        CasinoCloseSeq(&g_casino_seqs[1]);
    } else if (g_poker_double_pay >= 100 && g_poker_double_pay < 1000) {
        CasinoCloseSeq(&g_casino_seqs[1]);
    } else if (g_poker_double_pay >= 1000 && g_poker_double_pay < 1000000) {
        CasinoCloseSeq(&g_casino_seqs[1]);
    } else if (g_poker_double_pay >= 1000000) {
        CasinoCloseSeq(&g_casino_seqs[1]);
    }
}

/* Sprite 0x299 on for the first half of every n frames. */
void CasinoPokerDoubleBlink(u_char n)
{
    if (g_casino_timer % n == 0) {
        CasinoSpritesSetOn(0x299, 1, 1);
    }
    if (g_casino_timer % n == n / 2) {
        CasinoSpritesSetOn(0x299, 1, 0);
    }
}

void CasinoPokerDummy2(void)
{
}

void CasinoPokerDummy3(void)
{
}

/* The high-low call: circle or triangle says higher, square or cross lower. */
void CasinoPokerHiLoKeys(void)
{
    if (g_casino_pad_trig & PAD_CIRCLE || g_casino_pad_trig & PAD_TRIANGLE) {
        g_poker_hilo_guess = 0x16;
        CasinoTween(&D_800966F8, 0x140, 0, 0, 0, 0x20);
    } else if (g_casino_pad_trig & PAD_SQUARE || g_casino_pad_trig & PAD_CROSS) {
        g_poker_hilo_guess = 0x17;
        CasinoTween(&D_80096660, -0x140, 0, 0, 0, 0x20);
    }
}

void CasinoPokerDummy4(void)
{
}

/* Red or black: circle or triangle calls one colour, square or cross the other. */
void CasinoPokerColorKeys(void)
{
    if (g_casino_pad_trig & PAD_CIRCLE || g_casino_pad_trig & PAD_TRIANGLE) {
        g_poker_color_guess = 0;
        CasinoTween(&D_80096808, 0x140, 0, 0, 0, 0x20);
    } else if (g_casino_pad_trig & PAD_SQUARE || g_casino_pad_trig & PAD_CROSS) {
        g_poker_color_guess = 1;
        CasinoTween(&D_80096770, -0x140, 0, 0, 0, 0x20);
    }
}
