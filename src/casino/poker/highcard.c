/* Persona 1 (JP) - CASINO's video poker: the double-up dispatch and the
 * first double-up game, high card.
 *   0x80072D14 CasinoPokerDoublePlay
 *   0x80072DAC CasinoPokerHighRun
 *   0x80072EBC CasinoPokerHighOpen
 *   0x80072F78 CasinoPokerHighDeal
 *   0x800730DC CasinoPokerHighShow
 *   0x80073488 CasinoPokerHighPick
 *   0x80073624 CasinoPokerHighReveal
 *   0x800738E0 CasinoPokerHighWin
 *   0x80073AB0 CasinoPokerHighAgain
 *   0x80073C7C CasinoPokerHighLeave
 *   0x80073CE0 CasinoPokerHighCollect
 *   0x80073E28 CasinoPokerHighDraw
 *   0x80073F40 CasinoPokerHighLose
 *   0x80074138 CasinoPokerHighEnd
 *   0x800743CC CasinoPokerHighCursorInit
 *   0x80074420 CasinoPokerHighDealCards
 *
 * The dealer turns up one card - never the joker nor an ace, and always
 * one at least one of the other four beats - and the player picks one of
 * the four face down. Higher doubles the win (up to a million), equal is
 * a draw and deals again, lower loses it.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>
#include <persona/casino/casino.h>
#include <persona/casino/poker.h>

extern void CasinoTween();         /* (CasinoLayout *l, short dx, short dy, short dw, short dh, short frames) */
extern void CasinoSpritesSetOn();  /* (short first, short n, int on) */
extern void CasinoStartPalAnim(CasinoPalAnim *a);
extern void CasinoCursorRepeat(CasinoCursor *c, u_char mode, short rate);
extern void CasinoSetXform(CasinoXform *x);
extern void CasinoDrawObj(CasinoObj *o);
extern void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                            long sy, long sz);
extern void CasinoPlaySeq(short *h, u_long *seq, short vab);
extern void CasinoShowNumber(u_int n, CasinoLayout *digits, u_char count);
extern void CasinoSplitDigits(u_int n);
extern void CasinoPayStep(int *win);

extern void  CasinoPokerHiLoRun(void);
extern void  CasinoPokerColorRun(void);
extern void  func_800775CC(void);
extern void  CasinoPokerDoubleMaxed(void);
extern void  CasinoPokerDoubleNumbers(void);
extern void  CasinoPokerDoubleIntro(void);
extern void  CasinoPokerDoubleReadouts(void);
extern void  func_80077F38(void);
extern void  func_80078014(void);
extern void  func_80082658(u_char *out, int n, int range, int unique);
extern short func_80082454(u_char card);
extern void  func_800824CC(int i, u_char card, int face, int spr);
extern void  func_800825A4(u_char card, int spr);
extern u_char func_800827EC(u_char dealer, u_char pick);

extern u_char      D_800B0AF8[4];
extern CasinoXform g_casino_xforms[25];

void CasinoPokerHighRun(void);
void CasinoPokerHighOpen(void);
void CasinoPokerHighDeal(void);
void CasinoPokerHighShow(void);
void CasinoPokerHighPick(void);
void CasinoPokerHighReveal(void);
void CasinoPokerHighWin(void);
void CasinoPokerHighAgain(void);
void CasinoPokerHighLeave(void);
void CasinoPokerHighCollect(void);
void CasinoPokerHighDraw(void);
void CasinoPokerHighLose(void);
void CasinoPokerHighEnd(void);
void CasinoPokerHighCursorInit(void);
void CasinoPokerHighDealCards(void);

void CasinoPokerDoublePlay(void)
{
    switch (g_poker_double_game) {
    case 0:
        CasinoPokerHighRun();
        break;
    case 1:
        CasinoPokerHiLoRun();
        break;
    case 2:
        CasinoPokerColorRun();
        break;
    case 0x39:
        func_800775CC();
        break;
    }
}

void CasinoPokerHighRun(void)
{
    switch (g_poker_double_step) {
    case 0x11:
        CasinoPokerHighOpen();
        break;
    case 0x12:
        CasinoPokerHighDeal();
        break;
    case 0x34:
        CasinoPokerHighShow();
        break;
    case 0x15:
        CasinoPokerHighPick();
        break;
    case 0x35:
        CasinoPokerHighReveal();
        break;
    case 0x16:
        CasinoPokerHighWin();
        break;
    case 0x37:
        CasinoPokerHighAgain();
        break;
    case 0x38:
        CasinoPokerHighLeave();
        break;
    case 0x31:
        CasinoPokerHighCollect();
        break;
    case 0x19:
        CasinoPokerHighDraw();
        break;
    case 0x17:
        CasinoPokerHighLose();
        break;
    case 0x3A:
        CasinoPokerHighEnd();
        break;
    case 0x13:
        CasinoPokerDoubleMaxed();
        break;
    }
}

void CasinoPokerHighOpen(void)
{
    g_poker_double_shown = 0;
    g_poker_double_pay = g_poker_payout * 2;
    if (g_poker_double_pay > 999999) {
        g_poker_double_pay = 1000000;
    }
    CasinoPokerDoubleNumbers();
    CasinoTween(&D_8009651C, 0, -0x10, 0, 0, 1);
    CasinoTween(&D_800965E8, 0, -0x10, 0, 0, 1);
    g_casino_timer = -1;
    g_poker_double_step = 0x12;
}

void CasinoPokerHighDeal(void)
{
    CasinoPokerHighCursorInit();
    CasinoPokerHighDealCards();
    g_poker_held[0] = 0;
    g_poker_held[1] = 0;
    g_poker_held[2] = 0;
    g_poker_held[3] = 0;
    g_poker_held[4] = 0;
    D_800962E4->xform->rot.vy = -0x800;
    D_800962E4->xform->trans.vx = 0;
    D_800962E4->xform->trans.vy = 0;
    D_80096240->xform->rot.vy = -0x800;
    D_80096240->xform->trans.vx = 0;
    D_80096240->xform->trans.vy = 0;
    D_80095770->xform->rot.vy = 0;
    D_80095770->xform->rot.vx = 0x800;
    D_80095770->xform->trans.vx = 0;
    D_80095770->xform->trans.vy = 0x14;
    D_80096388->xform->rot.vy = -0x800;
    D_80096388->xform->trans.vx = 0;
    D_80096388->xform->trans.vy = 0;
    D_80096484->xform->rot.vy = -0x800;
    D_80096484->xform->trans.vx = 0;
    D_80096484->xform->trans.vy = 0;
    g_poker_double_step = 0x34;
    g_casino_timer = -1;
}

void CasinoPokerHighShow(void)
{
    if (g_casino_timer == 0) {
        CasinoStartAnim(&g_casino_objs[0], 0x20, 0, 0, 0, -0xC0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x20) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[0], 0x20, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        CasinoStartAnim(D_80095DD0, 0x20, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x21) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139E40, g_casino_main_vab);
    }
    if (g_casino_timer == 0x40) {
        CasinoStartAnim(&g_casino_objs[1], 0x20, 0, 0, 0, -0x120, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x60) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[2], 0x20, 0, 0, 0, -0x120, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x80) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[3], 0x20, 0, 0, 0, -0x120, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0xA0) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[4], 0x20, 0, 0, 0, -0x120, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0xC0) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
    }
    if (g_casino_timer == 0xC8) {
        CasinoStartAnim(&g_casino_objs[1], 8, 0, 0, 0, 0, -8, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0xD0 && !g_poker_double_shown) {
        CasinoPokerDoubleIntro();
    }
    if (g_casino_timer > 0xD8) {
        CasinoSpritesSetOn(0x3C, 0x1E, 0);
        if (!g_poker_double_shown) {
            CasinoPokerDoubleReadouts();
        }
        g_poker_double_shown = 1;
        g_poker_double_step = 0x15;
        g_casino_timer = -1;
    }
}

void CasinoPokerHighPick(void)
{
    CasinoCursorRepeat(&g_casino_cursor, 1, 10);
    if (g_casino_cursor.x != g_casino_cursor.prev_x) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[4 - g_casino_cursor.x], 8, 0, 0, 0, 0, -8, 0, 0, 0, 0);
        CasinoStartAnim(&g_casino_objs[4 - g_casino_cursor.prev_x], 8, 0, 0, 0, 0, 8, 0, 0, 0, 0);
    }
    if (((g_casino_pad_trig & PAD_CIRCLE) || (g_casino_pad_trig & PAD_SQUARE) || (g_casino_pad_trig & PAD_TRIANGLE) ||
         (g_casino_pad_trig & PAD_CROSS)) &&
        !D_800B0AF8[1]) {
        SsSeqStop(g_casino_seqs[4]);
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        g_poker_held[4 - g_casino_cursor.x] = 1;
        g_poker_double_step = 0x35;
        g_casino_timer = -1;
    }
}

void CasinoPokerHighReveal(void)
{
    short x[4] = { -0x60, -0x20, 0x20, 0x60 };
    int   k;
    s8    pick;
    u_char result;

    if (g_casino_timer == 0) {
        g_poker_flip_idx = 1;
        CasinoSpritesSetOn(0x3C, 0x1E, 1);
    }
    if (g_poker_flip_idx < POKER_CARDS) {
        if (!g_poker_held[g_poker_flip_idx]) {
            CasinoStartAnim(&g_casino_objs[g_poker_flip_idx], 0x10, 0, -0x800, 0, 0, 0, 0, 0, 0, 0);
            g_poker_held[g_poker_flip_idx] = 1;
        } else if (!g_casino_objs[g_poker_flip_idx].busy) {
            g_poker_flip_idx++;
        }
    }
    if (g_poker_flip_idx == POKER_CARDS) {
        CasinoStartAnim(&g_casino_objs[4 - g_casino_cursor.x], 0x20, 0, -0x800, 0, 0, 0, 0, 0, 0, 0);
        result = func_800827EC(g_poker_hand[0], g_poker_hand[4 - g_casino_cursor.x]);
        pick = g_casino_cursor.x;
        g_poker_double_step = g_poker_rank = result;
        k = 3 - pick;
        if (g_poker_double_step == 0x19) {
            D_80096388->xform->trans.vx = x[k];
            D_80096388->xform->trans.vy = 0x40;
            D_80096388->xform->trans.vy = 0x80;
            D_80096484->xform->trans.vx = x[k];
            D_80096484->xform->trans.vy = 0x40;
            D_80096484->xform->trans.vy = 0x80;
        } else if (g_poker_double_step == 0x16) {
            D_800962E4->xform->trans.vx = x[k];
            D_800962E4->xform->trans.vy = 0xA0;
        } else if (g_poker_double_step == 0x17) {
            D_80096240->xform->trans.vx = x[k];
            D_80096240->xform->trans.vy = 0xA0;
        }
        g_casino_timer = -1;
    }
}

void CasinoPokerHighWin(void)
{
    if (!g_poker_palanim4->on) {
        CasinoStartPalAnim(g_poker_palanim4);
    }
    if (g_casino_timer == 0) {
        g_poker_payout = g_poker_double_pay;
        if (g_poker_payout > 999999) {
            g_poker_payout = 1000000;
        }
        CasinoShowNumber(g_poker_payout, D_80095E94, 8);
    }
    if (g_casino_timer == 4) {
        CasinoStartAnim(D_800962E4, 0x10, 0, -0x800, 0, 0, -0x50, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x20) {
        func_80077F38();
        SsSeqSetVol(g_casino_seqs[15], 0x40, 0x40);
    }
    if (!SsIsEos(g_casino_seqs[1], 0) && g_casino_timer >= 0x20) {
        SsSeqSetVol(g_casino_seqs[15], 0x7F, 0x7F);
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

void CasinoPokerHighAgain(void)
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
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            CasinoTween(&D_800965E8, 0x140, 0, 0, 0, 0x20);
            g_poker_rank = g_poker_double_step = 0x38;
        } else if (g_casino_pad_trig & PAD_CROSS) {
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            CasinoTween(&D_8009651C, -0x140, 0, 0, 0, 0x20);
            g_poker_rank = g_poker_double_step = 0x31;
        }
    }
    if (g_poker_double_step != 0x37) {
        func_80078014();
        g_casino_timer = -1;
    }
}

void CasinoPokerHighLeave(void)
{
    if (g_casino_timer > 0x20) {
        CasinoTween(&D_8009651C, -0x140, 0, 0, 0, 0x20);
        g_casino_timer = -1;
        g_poker_double_step = 0x3A;
    }
}

void CasinoPokerHighCollect(void)
{
    if (g_casino_timer == 0) {
        CasinoSplitDigits(g_poker_payout);
    }
    if (!(g_casino_frame & 1) && g_poker_payout) {
        CasinoPayStep((int *)&g_poker_payout);
        if ((u_int)g_casino_money > 99999998) {
            g_poker_payout = 0;
            g_casino_money = 99999999;
        }
    }
    if (!g_poker_payout && g_casino_timer > 0x20) {
        CasinoTween(&D_800965E8, 0x140, 0, 0, 0, 0x20);
        g_casino_timer = -1;
        g_poker_double_step = 0x3A;
    }
    CasinoShowNumber(g_casino_money, D_80095FBC, 8);
    CasinoShowNumber(g_poker_payout, D_80095E94, 8);
    CasinoShowNumber(0, D_800960E4, 8);
}

void CasinoPokerHighDraw(void)
{
    if (!g_poker_palanim3->on) {
        CasinoStartPalAnim(g_poker_palanim3);
    }
    if (g_casino_timer == 4) {
        CasinoStartAnim(D_80096388, 0x10, 0, -0x800, 0, 0, -0x40, 0, 0, 0, 0);
        CasinoStartAnim(D_80096484, 0x20, 0, -0x800, 0, 0, -0x40, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x24) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x13A45C, g_casino_main_vab);
    }
    if (g_casino_timer > 0x78) {
        g_poker_double_step = 0x3A;
        g_casino_timer = -1;
    }
}

void CasinoPokerHighLose(void)
{
    if (g_casino_timer == 0x10) {
        CasinoStartAnim(D_80096240, 0x10, 0, -0x800, 0, 0, -0x50, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x20) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139C74, g_casino_vab);
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

void CasinoPokerHighEnd(void)
{
    int i;

    if (g_casino_timer == 0) {
        CasinoStartAnim(&g_casino_objs[4 - g_casino_cursor.x], 8, 0, 0, 0, 0, 8, 0, 0, 0, 0);
        for (i = 0; i < POKER_CARDS; i++) {
            CasinoStartAnim(&g_casino_objs[i], 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        }
        CasinoStartAnim(D_80095DD0, 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        if (g_poker_rank == 0x19) {
            CasinoStartAnim(D_80096388, 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
            CasinoStartAnim(D_80096484, 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        }
    }
    if (g_casino_timer == 0x10) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139E94, g_casino_main_vab);
        for (i = 0; i < POKER_CARDS; i++) {
            CasinoStartAnim(&g_casino_objs[i], 0x20, 0, 0, 0, -0x80 - (i + 1) * 64, 0, 0, 0, 0, 0);
        }
    }
    if (g_casino_timer > 0x30) {
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
        case 0x19:
            g_poker_double_step = 0x12;
            break;
        }
        g_casino_timer = -1;
    }
}

void CasinoPokerHighCursorInit(void)
{
    g_casino_cursor.prev_x = 3;
    g_casino_cursor.x = 3;
    g_casino_cursor.prev_y = 3;
    g_casino_cursor.y = 3;
    g_casino_cursor.w = 4;
    g_casino_cursor.h = 4;
    g_casino_cursor.wrap_x = 1;
    g_casino_cursor.wrap_y = 1;
}

void CasinoPokerHighDealCards(void)
{
    int    i;
    int    j;
    s8     n;
    u_char r;
    short  face;

    for (;;) {
        n = 0;
        func_80082658(g_poker_deck, 5, 0x35, 1);
        if (g_poker_deck[0] == 0x34) {
            continue;
        }
        r = g_poker_deck[0] % 13;
        if (!r) {
            continue;
        }
        for (j = 1; j < POKER_CARDS; j++) {
            if ((u_char)(g_poker_deck[j] % 13) >= r) {
                n++;
            }
        }
        if (n) {
            break;
        }
    }
    for (i = 0; i < POKER_CARDS; i++) {
        g_poker_hand[i] = g_poker_deck[i];
        face = func_80082454(g_poker_hand[i]);
        func_800824CC(i, g_poker_hand[i], face, i * 0x15 + 0x8D);
        func_800825A4(g_poker_hand[i], i * 0x15 + 0x8D);
        if (i == 0) {
            g_casino_xforms[i].trans.vx = 0xC0;
            g_casino_xforms[i].rot.vx = 0;
            g_casino_xforms[i].rot.vy = 0x800;
            g_casino_xforms[i].rot.vz = 0;
            g_casino_xforms[i].trans.vy = -0x30;
            g_casino_xforms[i].trans.vz = 0x200;
            g_casino_xforms[i].scale.vx = 0x1000;
            g_casino_xforms[i].scale.vy = 0x1000;
            g_casino_xforms[i].scale.vz = 0x1000;
        } else {
            g_casino_xforms[i].rot.vx = 0;
            g_casino_xforms[i].rot.vy = 0x800;
            g_casino_xforms[i].rot.vz = 0;
            g_casino_xforms[i].trans.vx = (i - 1) * 0x40 + 0xC0;
            g_casino_xforms[i].trans.vy = 0x48;
            g_casino_xforms[i].trans.vz = 0x200;
            g_casino_xforms[i].scale.vx = 0x1000;
            g_casino_xforms[i].scale.vy = 0x1000;
            g_casino_xforms[i].scale.vz = 0x1000;
        }
        CasinoSetXform(&g_casino_xforms[i]);
        CasinoDrawObj(&g_casino_objs[i]);
    }
}
