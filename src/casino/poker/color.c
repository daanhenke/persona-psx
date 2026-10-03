/* Persona 1 (JP) - CASINO's video poker: the third double-up game, red or
 * black, and the pieces the three games share.
 *   0x800760DC CasinoPokerColorRun
 *   0x800761DC CasinoPokerColorOpen
 *   0x800762C4 CasinoPokerColorDeal
 *   0x80076424 CasinoPokerColorShow
 *   0x80076568 CasinoPokerColorGuess
 *   0x800765DC CasinoPokerColorReveal
 *   0x8007668C CasinoPokerColorWin
 *   0x80076894 CasinoPokerDoubleMaxed
 *   0x80076910 CasinoPokerColorAgain
 *   0x80076AB0 CasinoPokerColorLeave
 *   0x80076B14 CasinoPokerColorCollect
 *   0x80076C80 CasinoPokerColorLose
 *   0x80076ECC CasinoPokerColorEnd
 *   0x80077004 CasinoPokerColorDealCard
 *   0x80077164 CasinoPokerDoubleIntro
 *   0x80077288 CasinoPokerDoubleReadouts
 *   0x8007756C CasinoPokerDoubleNumbers
 *
 * One card face down; the player calls its colour. The stake doubles on
 * a right call; at a million the game stops and pays (DoubleMaxed).
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>
#include <persona/casino/casino.h>
#include <persona/casino/poker.h>

extern void CasinoTween();        /* (CasinoLayout *l, short dx, short dy, short dw, short dh, short frames) */
extern void CasinoSpritesSetOn(); /* (short first, short n, int on) */
extern void CasinoShowLayout(CasinoLayout *l, u_char mode);
extern void CasinoStartPalAnim(CasinoPalAnim *a);
extern void CasinoSetXform(CasinoXform *x);
extern void CasinoDrawObj(CasinoObj *o);
extern void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                            long sy, long sz);
extern void CasinoPlaySeq(short *h, u_long *seq, short vab);
extern void CasinoShowNumber(u_int n, CasinoLayout *digits, u_char count);
extern void CasinoSplitDigits(u_int n);
extern void CasinoPayStep(int *win);

extern int   rand(void);
extern void  func_80077F38(void);
extern void  func_80078014(void);
extern void  func_80078094(int n);
extern void  func_800781FC(void);
extern short func_80082454(u_char card);
extern void  func_800824CC(int i, u_char card, int face, int spr);
extern void  func_800825A4(u_char card, int spr);
extern int   func_80082794(); /* (u_char card): its colour */

extern CasinoXform  g_casino_xforms[25];
extern CasinoLayout D_80096770; /* the two calls */
extern CasinoLayout D_80096808;

extern u_char g_poker_color_guess; /* 0 or 1, 0xFF none */

void CasinoPokerColorOpen(void);
void CasinoPokerColorDeal(void);
void CasinoPokerColorShow(void);
void CasinoPokerColorGuess(void);
void CasinoPokerColorReveal(void);
void CasinoPokerColorWin(void);
void CasinoPokerDoubleMaxed(void);
void CasinoPokerColorAgain(void);
void CasinoPokerColorLeave(void);
void CasinoPokerColorCollect(void);
void CasinoPokerColorLose(void);
void CasinoPokerColorEnd(void);
void CasinoPokerColorDealCard(void);
void CasinoPokerDoubleIntro(void);
void CasinoPokerDoubleReadouts(void);
void CasinoPokerDoubleNumbers(void);

void CasinoPokerColorRun(void)
{
    switch (g_poker_double_step) {
    case 0x11:
        CasinoPokerColorOpen();
        break;
    case 0x12:
        CasinoPokerColorDeal();
        break;
    case 0x34:
        CasinoPokerColorShow();
        break;
    case 0x35:
        CasinoPokerColorGuess();
        break;
    case 0x36:
        CasinoPokerColorReveal();
        break;
    case 0x16:
        CasinoPokerColorWin();
        break;
    case 0x37:
        CasinoPokerColorAgain();
        break;
    case 0x38:
        CasinoPokerColorLeave();
        break;
    case 0x31:
        CasinoPokerColorCollect();
        break;
    case 0x17:
        CasinoPokerColorLose();
        break;
    case 0x3A:
        CasinoPokerColorEnd();
        break;
    case 0x13:
        CasinoPokerDoubleMaxed();
        break;
    }
}

void CasinoPokerColorOpen(void)
{
    if (g_casino_timer == 0) {
        g_poker_double_shown = 0;
        g_poker_double_pay = g_poker_payout * 2;
        if (g_poker_double_pay > 999999) {
            g_poker_double_pay = 1000000;
        }
        CasinoPokerDoubleNumbers();
        CasinoTween(&D_8009651C, 0, 0x10, 0, 0, 1);
        CasinoTween(&D_800965E8, 0, 8, 0, 0, 1);
        CasinoShowLayout(&D_80096770, 0);
        CasinoShowLayout(&D_80096808, 0);
        g_casino_timer = -1;
        g_poker_double_step = 0x12;
    }
}

void CasinoPokerColorDeal(void)
{
    CasinoPokerColorDealCard();
    CasinoShowLayout(&D_80096770, 0);
    CasinoShowLayout(&D_80096808, 0);
    D_800962E4->xform->rot.vy = -0x800;
    D_800962E4->xform->trans.vx = 0;
    D_800962E4->xform->trans.vy = 0x50;
    D_80096240->xform->rot.vy = -0x800;
    D_80096240->xform->trans.vx = 0;
    D_80096240->xform->trans.vy = 0x50;
    D_80095770->xform->rot.vy = 0;
    D_80095770->xform->rot.vx = 0x800;
    D_80095770->xform->trans.vx = 0;
    D_80095770->xform->trans.vy = 0x60;
    D_80096388->xform->rot.vy = -0x800;
    D_80096388->xform->trans.vx = 0;
    D_80096388->xform->trans.vy = 0x50;
    D_80096484->xform->rot.vy = -0x800;
    D_80096484->xform->trans.vx = 0;
    D_80096484->xform->trans.vy = 0x50;
    g_poker_color_guess = 0xFF;
    g_poker_double_step = 0x34;
    g_casino_timer = -1;
}

void CasinoPokerColorShow(void)
{
    if (g_casino_timer == 0) {
        CasinoStartAnim(&g_casino_objs[0], 0x20, 0, 0, 0, -0x140, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x20) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
        if (!g_poker_double_shown) {
            CasinoPokerDoubleIntro();
        }
        CasinoTween(&D_80096770, 0x140, 0, 0, 0, 0x20);
        CasinoTween(&D_80096808, -0x140, 0, 0, 0, 0x20);
    }
    if (g_casino_timer > 0x40) {
        if (!g_poker_double_shown) {
            CasinoPokerDoubleReadouts();
        }
        g_poker_double_shown = 1;
        g_poker_double_step = 0x35;
        g_casino_timer = -1;
    }
}

void CasinoPokerColorGuess(void)
{
    func_800781FC();
    if (g_poker_color_guess != 0xFF) {
        SsSeqStop(g_casino_seqs[4]);
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        g_poker_double_step = 0x36;
        g_casino_timer = -1;
    }
}

void CasinoPokerColorReveal(void)
{
    u_char guess;

    CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139E40, g_casino_main_vab);
    CasinoStartAnim(&g_casino_objs[0], 0x20, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
    guess = g_poker_color_guess;
    if (guess == func_80082794(g_poker_hand[0])) {
        g_poker_double_step = g_poker_rank = 0x16;
    } else {
        g_poker_double_step = g_poker_rank = 0x17;
    }
    g_casino_timer = -1;
}

void CasinoPokerColorWin(void)
{
    if (!g_poker_palanim4->on) {
        CasinoStartPalAnim(g_poker_palanim4);
    }
    if (g_casino_timer == 4) {
        CasinoStartAnim(D_800962E4, 0x10, 0, -0x800, 0, 0, -0x40, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x10) {
        g_poker_payout = g_poker_double_pay;
        if (g_poker_payout > 999999) {
            g_poker_payout = 1000000;
        }
        CasinoShowNumber(g_poker_payout, D_80095E94, 8);
        func_80077F38();
        SsSeqSetVol(g_casino_seqs[15], 0x40, 0x40);
    }
    if (!SsIsEos(g_casino_seqs[1], 0) && g_casino_timer >= 0x10) {
        SsSeqSetVol(g_casino_seqs[15], 0x7F, 0x7F);
        if (g_poker_color_guess == 0) {
            CasinoTween(&D_80096770, -0x140, 0, 0, 0, 0x20);
        } else if (g_poker_color_guess == 1) {
            CasinoTween(&D_80096808, 0x140, 0, 0, 0, 0x20);
        }
        CasinoStartAnim(D_800962E4, 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        if (g_poker_payout > 999999) {
            g_poker_double_step = 0x13;
            g_poker_rank = 0x31;
        } else {
            g_poker_double_step = 0x37;
        }
        g_casino_timer = -1;
    }
}

/* The stake reached a million: say so, and pay it out on a key. */
void CasinoPokerDoubleMaxed(void)
{
    func_80078094(0x10);
    if (((g_casino_pad_trig & PAD_CIRCLE) || (g_casino_pad_trig & PAD_SQUARE) || (g_casino_pad_trig & PAD_TRIANGLE) ||
         (g_casino_pad_trig & PAD_CROSS)) &&
        g_casino_timer > 0x78) {
        g_poker_double_step = 0x31;
        g_casino_timer = -1;
    }
}

void CasinoPokerColorAgain(void)
{
    if (g_casino_timer == 0) {
        g_poker_double_pay = g_poker_payout * 2;
        if (g_poker_double_pay > 999999) {
            g_poker_double_pay = 1000000;
        }
        CasinoShowNumber(g_poker_double_pay, D_800960E4, 8);
        CasinoTween(&D_8009651C, 0x140, 0, 0, 0, 0x20);
        CasinoTween(&D_800965E8, -0x140, 0, 0, 0, 0x20);
    }
    if (g_casino_timer > 0x20) {
        if (g_casino_pad_trig & PAD_CIRCLE) {
            CasinoTween(&D_800965E8, 0x140, 0, 0, 0, 0x20);
            g_poker_rank = g_poker_double_step = 0x38;
        } else if (g_casino_pad_trig & PAD_CROSS) {
            CasinoTween(&D_8009651C, -0x140, 0, 0, 0, 0x20);
            g_poker_rank = g_poker_double_step = 0x31;
        }
    }
    if (g_poker_double_step != 0x37) {
        func_80078014();
        SsSeqStop(g_casino_seqs[4]);
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        g_casino_timer = -1;
    }
}

void CasinoPokerColorLeave(void)
{
    if (g_casino_timer > 0x14) {
        CasinoTween(&D_8009651C, -0x140, 0, 0, 0, 0x20);
        g_casino_timer = -1;
        g_poker_double_step = 0x3A;
    }
}

void CasinoPokerColorCollect(void)
{
    if (g_casino_timer == 0) {
        CasinoSplitDigits(g_poker_payout);
    }
    if (!(g_casino_frame & 1)) {
        CasinoPayStep((int *)&g_poker_payout);
        if ((u_int)g_casino_money > 99999998) {
            g_poker_payout = 0;
            g_casino_money = 99999999;
        }
    }
    if (!g_poker_payout) {
        if (g_poker_color_guess == 0) {
            CasinoTween(&D_80096770, -0x140, 0, 0, 0, 0x20);
        } else if (g_poker_color_guess == 1) {
            CasinoTween(&D_80096808, 0x140, 0, 0, 0, 0x20);
        }
        CasinoTween(&D_800965E8, 0x140, 0, 0, 0, 0x20);
        g_casino_timer = -1;
        g_poker_double_step = 0x3A;
    }
    CasinoShowNumber(g_casino_money, D_80095FBC, 8);
    CasinoShowNumber(g_poker_payout, D_80095E94, 8);
    CasinoShowNumber(0, D_800960E4, 8);
}

void CasinoPokerColorLose(void)
{
    if (g_casino_timer == 0x10) {
        CasinoStartAnim(D_80096240, 0x10, 0, -0x800, 0, 0, -0x40, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x20) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139C74, g_casino_vab);
        if (g_poker_color_guess == 0) {
            CasinoTween(&D_80096770, 0, 0, 0, -0x10, 0x10);
        } else if (g_poker_color_guess == 1) {
            CasinoTween(&D_80096808, 0, 0, 0, -0x10, 0x10);
        }
        CasinoStartAnim(D_80095770, 0x20, -0x800, 0, 0, 0, 0, 0, 0, 0, 0);
        g_poker_payout = 0;
        g_poker_double_pay = 0;
        CasinoShowNumber(g_poker_double_pay, D_800960E4, 8);
        CasinoShowNumber(g_poker_payout, D_80095E94, 8);
    }
    if (g_casino_timer == 0x40) {
        CasinoPlaySeq(&g_casino_seqs[2], (u_long *)0x139F60, g_casino_main_vab);
    }
    if (((g_casino_pad_trig & PAD_TRIANGLE) && g_casino_timer > 0x40) || g_casino_timer > 0x78) {
        g_casino_timer = -1;
        CasinoStartAnim(D_80096240, 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        CasinoStartAnim(D_80095770, 0x10, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
        g_poker_double_step = 0x3A;
    }
}

void CasinoPokerColorEnd(void)
{
    if (g_casino_timer == 0) {
        CasinoStartAnim(&g_casino_objs[0], 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x10) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139E94, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[0], 0x10, 0, 0, 0, -0xC0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer > 0x20) {
        switch (g_poker_rank) {
        case 0x38:
            g_poker_double_step = 0x12;
            break;
        case 0x31:
            g_poker_double_game = 0x39;
            break;
        case 0x17:
            g_poker_double_game = 0x39;
            break;
        }
        g_casino_timer = -1;
    }
}

void CasinoPokerColorDealCard(void)
{
    short face;

    g_poker_hand[0] = g_poker_deck[0] = rand() % 52;
    if (g_poker_hand[0] == 0x34) {
        g_poker_hand[0] = rand() % 52;
    }
    face = func_80082454(g_poker_hand[0]);
    func_800824CC(0, g_poker_hand[0], face, 0x8D);
    func_800825A4(g_poker_hand[0], 0x8D);
    g_casino_xforms[0].rot.vx = 0;
    g_casino_xforms[0].rot.vy = 0x800;
    g_casino_xforms[0].rot.vz = 0;
    g_casino_xforms[0].trans.vx = 0x140;
    g_casino_xforms[0].trans.vy = 0;
    g_casino_xforms[0].trans.vz = 0x200;
    g_casino_xforms[0].scale.vx = 0x1000;
    g_casino_xforms[0].scale.vy = 0x1000;
    g_casino_xforms[0].scale.vz = 0x1000;
    CasinoSetXform(&g_casino_xforms[0]);
    CasinoDrawObj(&g_casino_objs[0]);
}

void CasinoPokerDoubleIntro(void)
{
    CasinoSpritesSetOn(0x295, 1, 1);
    CasinoSpritesSetOn(0x296, 1, 1);
    CasinoSpritesSetOn(0x297, 1, 1);
    CasinoSpritesSetOn(0x299, 1, 1);
    CasinoSpritesSetOn(0x29A, 8, 1);
    CasinoSpritesSetOn(0x2A2, 8, 1);
    CasinoSpritesSetOn(0x2AA, 8, 1);
    CasinoTween(&D_80095CF8, 0, -0x10, 0, 0x20, 0x10);
    CasinoTween(&D_80095D30, 0, -0x10, 0, 0x20, 0x10);
    CasinoTween(&D_80095D68, 0, -0x10, 0, 0x20, 0x10);
    CasinoTween(&D_80095E14, 0, 0, 0, 8, 8);
}

void CasinoPokerDoubleReadouts(void)
{
    CasinoTween(&D_80095E94[0], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095E94[1], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095E94[2], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095E94[3], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095E94[4], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095E94[5], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095E94[6], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095E94[7], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095FBC[0], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095FBC[1], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095FBC[2], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095FBC[3], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095FBC[4], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095FBC[5], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095FBC[6], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_80095FBC[7], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_800960E4[0], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_800960E4[1], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_800960E4[2], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_800960E4[3], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_800960E4[4], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_800960E4[5], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_800960E4[6], 0, -0x10, 0, 0x10, 0x10);
    CasinoTween(&D_800960E4[7], 0, -0x10, 0, 0x10, 0x10);
}

void CasinoPokerDoubleNumbers(void)
{
    CasinoShowNumber(g_casino_money, D_80095FBC, 8);
    CasinoShowNumber(g_poker_double_pay, D_800960E4, 8);
    CasinoShowNumber(g_poker_payout, D_80095E94, 8);
}
