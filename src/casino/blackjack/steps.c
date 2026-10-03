/* Persona 1 (JP) - CASINO's blackjack: the step machine, from the table
 * opening to the player's first choice.
 *   0x80078294 CasinoBjRun
 *   0x800783E0 CasinoBjOpen
 *   0x80078560 CasinoBjEnter
 *   0x80078704 CasinoBjBet
 *   0x800789C8 CasinoBjStartHand
 *   0x80078B7C CasinoBjDeal
 *   0x80079028 CasinoBjInsure
 *   0x800792A8 CasinoBjInsured
 *   0x80079764 CasinoBjNoInsurance
 *   0x80079880 CasinoBjChoose
 *
 * As in poker, each step runs once a frame on g_casino_timer and hands over
 * by setting the next step and the timer to -1.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>
#include <persona/casino/casino.h>
#include <persona/casino/blackjack.h>

extern void CasinoGame2LoadVram0(void);
extern void CasinoGame2LoadVram1(void);
extern void CasinoGame2LoadSound(void);
extern void CasinoFade(short first, short count, u_char r, u_char g, u_char b, short frames);
extern void CasinoStartPalAnim(CasinoPalAnim *a);
extern void CasinoSpritesSetOn(); /* (short first, short n, int on) */
extern void CasinoTween();        /* (CasinoLayout *l, short dx, short dy, short dw, short dh, short frames) */
extern void CasinoCursorClear(CasinoCursor *c);
extern void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                            long sy, long sz);
extern void CasinoPlaySeq(short *h, u_long *seq, short vab);
extern void CasinoObjSetOn(CasinoObj *o, short first, short n, u_char on);
extern void CasinoShowNumber(u_int n, CasinoLayout *digits, u_char count);
extern void CasinoShowCents(u_int n, CasinoLayout *digits, u_char count);
extern void CasinoSplitDigits(u_int n);
extern void CasinoPayStep(int *win);

extern u_char g_casino_wrapped;
extern s8     g_casino_key_fire[4];
extern s8     g_casino_key_hold[4];

extern void   func_8007DA00(void);
extern void   func_8007DAFC(void);
extern void   func_8007DBC8(void);
extern void   func_8007E30C(void);
extern void   func_8007E570(void);
extern void   func_8007E614(u_char bet);
extern void   func_8007E878(u_char who, u_char face, u_char *cards, u_char *count);
extern void   func_8007EB20(u_char kind, u_char value, CasinoLayout *l, u_char *n, CasinoLayoutDef *defs, u_char sound,
                            u_short x);
extern void   func_8007EF28(u_char n);
extern void   func_8007F00C(u_char menu);
extern void   func_8007F160(u_char menu, u_char dir, int step);
extern void   func_8007ADD8(void);
extern void   func_80080270(void);
extern void   func_800808D8(void);
extern void   func_80080A78(void);
extern void   func_80080E0C(void);
extern void   func_80080F1C(void);
extern void   func_80081224(u_char split, u_char dbl);
extern u_char func_80081DC0(u_char *cards);
extern void   func_8008205C(u_char *cards, u_char *total);
extern void   func_80082658(u_char *out, int n, int range, int unique);
extern void   func_800799F0(void);

void CasinoBjOpen(void);
void CasinoBjEnter(void);
void CasinoBjBet(void);
void CasinoBjStartHand(void);
void CasinoBjDeal(void);
void CasinoBjInsure(void);
void CasinoBjInsured(void);
void CasinoBjNoInsurance(void);
void CasinoBjChoose(void);

void CasinoBjRun(void)
{
    switch (g_casino_step) {
    case BJ_OPEN:
        CasinoBjOpen();
        break;
    case BJ_ENTER:
        CasinoBjEnter();
        break;
    case BJ_BET:
        CasinoBjBet();
        break;
    case BJ_START_HAND:
        CasinoBjStartHand();
        break;
    case BJ_DEAL:
        CasinoBjDeal();
        break;
    case BJ_CHOOSE:
        CasinoBjChoose();
        break;
    case BJ_INSURE:
        CasinoBjInsure();
        break;
    case BJ_INSURED:
        CasinoBjInsured();
        break;
    case BJ_NO_INSURANCE:
        CasinoBjNoInsurance();
        break;
    case BJ_PLAY:
        func_800799F0();
        break;
    case 0x77:
        func_8007ADD8();
        break;
    case 0x3C:
        func_80080270();
        break;
    case 0x3D:
        func_800808D8();
        break;
    case 0x7A:
        func_80080A78();
        break;
    case 0x7B:
        func_80080E0C();
        break;
    }
    if (g_casino_step != BJ_OPEN) {
        func_80080F1C();
    }
}

void CasinoBjOpen(void)
{
    if (g_casino_timer == 0) {
        CasinoGame2LoadVram0();
    }
    if (g_casino_timer == 1) {
        CasinoGame2LoadVram1();
    }
    if (g_casino_timer == 2) {
        func_8007DA00();
        CasinoGame2LoadSound();
        func_8007DBC8();
        CasinoFade(0, 0x3C1, 0x80, 0x80, 0x80, 0x20);
    }
    if (g_casino_timer == 4) {
        CasinoShowNumber(g_casino_money, g_bj_money_digits, 8);
        CasinoShowCents(g_bj_jackpot, g_bj_jackpot_digits, 6);
        CasinoShowNumber(g_casino_bet, g_bj_bet_digits, 2);
        CasinoShowNumber(g_casino_win, g_bj_win_digits, 4);
        func_8007E614(g_casino_bet);
    }
    if (g_casino_timer >= 4 && !g_bj_palanim0->on) {
        CasinoStartPalAnim(g_bj_palanim0);
    }
    if (g_casino_timer == 0x24) {
        g_casino_timer = -1;
        g_casino_step = BJ_ENTER;
    }
}

void CasinoBjEnter(void)
{
    if (!g_bj_palanim0->on) {
        CasinoStartPalAnim(g_bj_palanim0);
    }
    if (g_casino_timer == 0) {
        if (g_casino_wrapped) {
            SsSeqStop(g_casino_seqs[15]);
        }
        SsSeqPlay(g_casino_seqs[15], 1, 1);
        CasinoCursorClear((CasinoCursor *)g_casino_key_fire);
        CasinoCursorClear((CasinoCursor *)g_casino_key_hold);
        func_8007DAFC();
        if (g_bj_reshuffle == 1) {
            func_80082658(g_casino_deck, 0xD0, 0x34, 4);
            g_bj_shoe_pos = 0;
            g_bj_reshuffle = 0;
        }
        CasinoSpritesSetOn(0x8C, 0x1F4, 0);
        CasinoSpritesSetOn(0x1C, 2, 0);
        CasinoSpritesSetOn(0x26, 2, 0);
        CasinoSpritesSetOn(0x338, 0x30, 1);
        CasinoTween(D_8009B81C[0].l, 0x140, 0, 0, 0, 0x20);
        CasinoTween(D_8009B874[0].l, -0x140, 0, 0, 0, 0x20);
        CASINO_LAMPS.b0 = 1;
    }
    if (g_casino_timer > 0x20) {
        g_casino_step = BJ_BET;
    }
}

void CasinoBjBet(void)
{
    if (!g_bj_palanim0->on) {
        CasinoStartPalAnim(g_bj_palanim0);
    }
    if (!g_casino_max_bet) {
        func_8007E30C();
    } else if (g_casino_money && g_casino_bet < 10) {
        g_casino_bet++;
        g_casino_money--;
        if (g_casino_bet % 3 == 0) {
            SsSeqStop(g_casino_seqs[6]);
        }
        SsSeqPlay(g_casino_seqs[6], 1, 1);
    }
    if (g_casino_bet == 10 || (g_casino_bet != 0 && g_casino_money == 0)) {
        g_casino_bet_done = 1;
    }
    if (g_casino_bet != g_casino_bet_shown && g_casino_timer != 0) {
        func_8007E570();
        func_8007E614(g_casino_bet);
    }
    if (g_casino_bet_done == 1) {
        g_casino_step = BJ_START_HAND;
        g_casino_timer = -1;
        g_casino_max_bet = 0;
        CASINO_LAMPS.b2 = 0;
        CASINO_LAMPS.b4 = 0;
        g_casino_jackpot_add = g_casino_bet;
    }
    if (g_casino_quit == 1) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x138880, g_casino_main_vab);
        if (g_casino_bet) {
            SsSeqStop(g_casino_seqs[5]);
            SsSeqPlay(g_casino_seqs[5], 1, 1);
            g_casino_money += g_casino_bet;
            g_casino_bet = 0;
            func_8007E614(0);
        }
        CASINO_LAMPS.b0 = 0;
        CASINO_LAMPS.b2 = 0;
        CASINO_LAMPS.b4 = 0;
        CasinoFade(0, 0x3C1, 0, 0, 0, 0x20);
        g_casino_timer = -1;
        g_casino_game = CASINO_GAME_LEAVE;
    }
}

void CasinoBjStartHand(void)
{
    if (g_casino_timer == 0) {
        CasinoTween(D_8009B6D8[0].l, -0x140, 0, 0, 0, 0x20);
        CasinoStartAnim(D_8009B7B0, 0x20, 0, 0, 0, 0x140, 0, 0, 0, 0, 0);
        CasinoTween(D_8009B81C[0].l, -0x140, 0, 0, 0, 0x20);
        CasinoTween(D_8009B874[0].l, 0x140, 0, 0, 0, 0x20);
        CASINO_LAMPS.b0 = 0;
    }
    if (g_casino_timer > 8) {
        func_8007E878(0, 0, g_bj_cards[0], &g_bj_count[0]);
        func_8007E878(3, 0, g_bj_dealer, &g_bj_dealer_count);
        func_8007E878(0, 0, g_bj_cards[0], &g_bj_count[0]);
        func_8007E878(3, 0, g_bj_dealer, &g_bj_dealer_count);
        func_8008205C(g_bj_cards[0], g_bj_total);
        g_bj_natural[0] = func_80081DC0(g_bj_cards[0]);
        g_casino_step = BJ_DEAL;
        g_casino_timer = -1;
    }
}

void CasinoBjDeal(void)
{
    short k;

    if (g_casino_timer % 32 == 0) {
        k = 0x18 - g_casino_timer / 32;
        if (g_casino_timer <= 0x60) {
            if (g_casino_timer != 0) {
                CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x138830, g_casino_main_vab);
            }
            CasinoStartAnim(&g_casino_objs[k], 0x20, 0, 0, 0, -0x140, 0, 0, 0, 0, 0);
        }
        if (g_casino_timer != 0 && g_casino_timer < 0x80) {
            CasinoStartAnim(&g_casino_objs[k + 1], 0x20, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        }
        if (g_casino_timer <= 0x20) {
            CasinoStartAnim(&g_casino_objs[k], 4, 0, 0, 0, -4, 0, 0, 0, 0, 0);
        }
    }
    if (g_casino_timer == 0x80) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x138830, g_casino_main_vab);
        CasinoSpritesSetOn(0x2B0, 3, 1);
        CasinoTween(D_8009AD88[0].l, 0, -8, 0, 0x10, 8);
        CasinoTween(D_8009AD88[1].l, 0, -8, 0, 0x10, 8);
        CasinoSpritesSetOn(0x2AD, 3, 1);
        CasinoSpritesSetOn(0x2AD, 3, 1);
        CasinoSpritesSetOn(0x2B0, 3, 1);
        CasinoTween(D_8009ACE4[0].l, 0, -8, 0, 0x10, 8);
        CasinoTween(D_8009ACE4[1].l, 0, -8, 0, 0x10, 8);
    }
    if (g_casino_timer == 0x89) {
        CasinoSpritesSetOn(0x2C3, 0xA, 1);
        func_8007EB20(g_bj_natural[0], g_bj_total[1], D_8009B2B4[1].l, &g_bj_label_n[1], D_8009AD88, 1, 8);
        CasinoSpritesSetOn(0x2B9, 0xA, 1);
        func_8007EB20(0xE, 0, D_8009B2B4[0].l, &g_bj_label_n[0], D_8009ACE4, 1, 8);
    }
    if (g_casino_timer > 0x89) {
        CasinoObjSetOn(&g_casino_objs[24], 2, 0x11, 0);
        CasinoObjSetOn(&g_casino_objs[23], 2, 0x11, 0);
        CasinoSpritesSetOn(0x338, 0x30, 0);
        switch (g_bj_natural[0]) {
        case BJ_HAND_PLAIN:
            if (g_bj_dealer[0] % 13 == 0 && g_casino_money >= g_casino_bet / 2 && g_casino_bet != 1) {
                g_casino_step = BJ_INSURE;
            } else {
                g_casino_step = BJ_CHOOSE;
            }
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            g_casino_step = BJ_PLAY;
            g_bj_play_step = 0x71;
            break;
        case BJ_HAND_NATURAL:
            if (g_bj_dealer[0] % 13 == 0 && g_casino_money >= g_casino_bet / 2) {
                g_casino_step = BJ_INSURE;
            } else {
                g_casino_step = BJ_PLAY;
                g_bj_play_step = 0x71;
            }
            break;
        }
        g_casino_timer = -1;
    }
}

void CasinoBjInsure(void)
{
    if (g_casino_timer == 0) {
        CasinoSpritesSetOn(0x39B, 1, 1);
        CasinoSpritesSetOn(0x39C, 0x15, 1);
        CasinoTween(D_8009B97C[0].l, -0x58, -0x28, 0xB0, 0x50, 8);
    }
    if (g_casino_timer == 8) {
        CasinoTween(D_8009B9E4[0].l, 0, -8, 0, 0x10, 8);
    }
    if (g_casino_timer == 0x10) {
        CasinoTween(D_8009BA64[0].l, 0, -8, 0, 0x10, 8);
    }
    if (g_casino_timer == 0x18) {
        CasinoTween(D_8009BA64[1].l, 0, -8, 0, 0x10, 8);
    }
    if (g_casino_timer > 0x20) {
        if (g_casino_pad_trig & PAD_TRIANGLE || g_casino_pad_trig & PAD_CIRCLE) {
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            CasinoTween(D_8009BA64[1].l, 0, 8, 0, -0x10, 8);
            CasinoTween(D_8009BA64[0].l, 0, 8, 0, 0, 8);
            g_casino_timer = -1;
            g_bj_insurance = g_casino_bet / 2;
            g_casino_step = BJ_INSURED;
            g_casino_money -= g_bj_insurance;
            g_casino_jackpot_add = g_bj_insurance;
        } else if (g_casino_pad_trig & PAD_SQUARE || g_casino_pad_trig & PAD_CROSS) {
            SsSeqStop(g_casino_seqs[5]);
            SsSeqPlay(g_casino_seqs[5], 1, 1);
            CasinoTween(D_8009BA64[0].l, 0, 8, 0, -0x10, 4);
            g_casino_timer = -1;
            g_casino_step = BJ_NO_INSURANCE;
        }
    }
}

void CasinoBjInsured(void)
{
    if (g_casino_timer == 8) {
        if (g_bj_dealer[1] % 13 >= 9) {
            g_bj_dealer_bj = 1;
        } else {
            g_bj_dealer_bj = 0;
        }
        CasinoStartAnim(&g_casino_objs[21], 8, 0, 0x200, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x48) {
        CasinoStartAnim(&g_casino_objs[21], 8, 0, -0x200, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer <= 0x60 && g_casino_timer % 16 == 0) {
        CasinoSpritesSetOn(g_casino_timer / 16 + 0x3BB, 1, 1);
    }
    if (g_casino_timer == 0x70) {
        CasinoSpritesSetOn(0x3BB, 6, 0);
    }
    if (g_casino_timer >= 0x80) {
        if (!g_bj_dealer_bj) {
            if (g_casino_timer == 0x80) {
                CasinoTween(D_8009B9E4[0].l, 0, 8, 0, -0x10, 4);
                CasinoSpritesSetOn(0x3B1, 0xA, 1);
            }
            if (g_casino_timer == 0xB0) {
                CasinoSpritesSetOn(0x3B1, 0xA, 0);
                CasinoTween(D_8009BA64[0].l, 0, 0, 0, -0x10, 4);
                CasinoTween(D_8009B97C[0].l, 0x58, 0x28, -0xB0, -0x50, 8);
                SsSeqStop(g_casino_seqs[5]);
                SsSeqPlay(g_casino_seqs[5], 1, 1);
            }
            if (g_casino_timer == 0xC0) {
                CasinoSpritesSetOn(0x39B, 1, 0);
                CasinoSpritesSetOn(0x39C, 0x15, 0);
                g_bj_insurance = 0;
                if (g_bj_natural[0] == BJ_HAND_NATURAL) {
                    g_casino_step = BJ_PLAY;
                    g_bj_play_step = 0x71;
                } else {
                    g_casino_step = BJ_CHOOSE;
                }
                g_casino_timer = -1;
            }
        } else {
            if (g_casino_timer == 0x80) {
                g_casino_win = g_bj_insurance * 3;
                CasinoSplitDigits(g_casino_win);
                CasinoStartAnim(&g_casino_objs[21], 0x20, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
                CasinoPlaySeq(&g_casino_seqs[2], (u_long *)0x1384C4, g_casino_vab);
                func_8007EB20(8, g_bj_dealer_total[1], D_8009B2B4[0].l, &g_bj_label_n[0], D_8009ACE4, 1, 8);
                CasinoTween(D_8009B9E4[0].l, 0, 8, 0, -0x10, 4);
                CasinoTween(D_8009BA64[0].l, 0, 0, 0, -0x10, 4);
                CasinoTween(D_8009B97C[0].l, 0x58, 0x28, -0xB0, -0x50, 8);
            }
            if (g_casino_timer >= 0xC0) {
                if (!(g_casino_frame & 1)) {
                    CasinoPayStep(&g_casino_win);
                }
                if (g_casino_money > 99999998) {
                    g_casino_win = 0;
                    g_casino_money = 99999999;
                }
                if (!g_casino_win) {
                    g_casino_timer = -1;
                    g_casino_step = BJ_PLAY;
                    g_bj_play_step = 0x30;
                }
            }
        }
    }
}

void CasinoBjNoInsurance(void)
{
    if (g_casino_timer == 4) {
        CasinoTween(D_8009BA64[1].l, 0, 8, 0, -0x10, 4);
        CasinoTween(D_8009B9E4[0].l, 0, 8, 0, -0x10, 4);
        CasinoTween(D_8009B97C[0].l, 0x58, 0x28, -0xB0, -0x50, 8);
    }
    if (g_casino_timer == 0xC) {
        CasinoSpritesSetOn(0x39B, 1, 0);
        CasinoSpritesSetOn(0x39C, 0x15, 0);
        if (g_bj_natural[0] == BJ_HAND_NATURAL) {
            g_casino_step = BJ_PLAY;
            g_bj_play_step = 0x71;
        } else {
            g_casino_step = BJ_CHOOSE;
        }
        g_casino_timer = -1;
    }
}

/* Hit, stand, and double or split when the money covers another bet: a
   pair offers the split, anything else the double. */
void CasinoBjChoose(void)
{
    u_char split;
    u_char dbl;
    u_char pair;
    int    afford;

    split = 0;
    dbl = 0;
    pair = g_bj_cards[0][0] % 13 == g_bj_cards[0][1] % 13;
    afford = g_casino_money >= g_casino_bet;
    if (afford) {
        if (pair) {
            split = 1;
        } else {
            dbl = 1;
        }
    }
    if (g_casino_timer == 0) {
        func_8007EF28(0);
        if (split) {
            func_8007F00C(0x73);
        } else if (dbl) {
            func_8007F00C(0x72);
        } else {
            func_8007F00C(0x70);
        }
    }
    if (g_casino_timer < 8) {
        if (split) {
            func_8007F160(0x73, 1, g_casino_timer);
        } else if (dbl) {
            func_8007F160(0x72, 1, g_casino_timer);
        } else {
            func_8007F160(0x70, 1, g_casino_timer);
        }
    }
    if (g_casino_timer > 8) {
        func_80081224(split, dbl);
    }
}
