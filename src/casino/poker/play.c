/* Persona 1 (JP) - CASINO's video poker: what the steps call while a hand
 * is played - the HUD, the bet's lamps and the pay table's lit row, the
 * cursor over the cards, holding them, and the double-up panels.
 *   0x80071074 CasinoPokerDoublePanels
 *   0x800713D8 CasinoPokerShowCard
 *   0x8007145C CasinoPokerCycleCard
 *   0x800715C8 CasinoPokerHud
 *   0x800718EC CasinoPokerPlaceBet
 *   0x80071948 CasinoPokerQuit
 *   0x80071A7C CasinoPokerMoveCursor
 *   0x80071BB8 CasinoPokerHoldKeys
 *   0x80071CFC CasinoPokerDummy
 *   0x80071D04 CasinoPokerStartDraw
 *   0x80071ECC CasinoPokerLightRow
 *   0x80071F88 CasinoPokerPayTable
 *   0x80072314 CasinoPokerBetDigits
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>
#include <persona/casino/casino.h>
#include <persona/casino/poker.h>

extern void CasinoQueueCluts(); /* (short first, short n, int clut), called unprototyped */
extern void CasinoTween();      /* (CasinoLayout *l, short dx, short dy, short dw, short dh, short frames) */
extern void CasinoSpritesSetOn(); /* (short first, short n, int on) */
extern void CasinoFade(short first, short count, u_char r, u_char g, u_char b, short frames);
extern void CasinoTexLayout(CasinoLayout *l);
extern void CasinoSetXform(CasinoXform *x);
extern void CasinoDrawObj(CasinoObj *o);
extern void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                            long sy, long sz);
extern void CasinoPlaySeq(short *h, u_long *seq, short vab);
extern void CasinoShowNumber(u_int n, CasinoLayout *digits, u_char count);
extern void CasinoShowCents(u_int n, CasinoLayout *digits, u_char count);

extern short func_80082454(u_char card);
extern void  func_800824CC(int i, u_char card, int face, int spr);
extern void  func_800825A4(u_char card, int spr);

extern int         g_casino_money_shown;
extern CasinoXform g_casino_xforms[25];

void CasinoPokerLightRow();
void CasinoPokerPayTable();
void CasinoPokerBetDigits();

void CasinoPokerDoublePanels(void)
{
    switch (g_poker_double_game) {
    case 0:
        switch (g_casino_timer) {
        case 1:
            CasinoTween(&D_80095BF4, 8, 8, -0x10, -0x10, 8);
            CasinoTween(&D_80095CC0, 8, 8, -0x10, -0x10, 8);
            break;
        case 8:
            CasinoSpritesSetOn(0x27F, 0xB, 0);
            CasinoSpritesSetOn(0x28A, 0xB, 0);
            CasinoTween(&D_80095B28, 0, -8, 0, 0x20, 8);
            CasinoPlaySeq(&g_casino_seqs[3], (u_long *)0x139DDC, g_casino_main_vab);
            break;
        case 0x10:
            CasinoTween(&D_80095B28, 0, -0x20, 0, -0x20, 0x10);
            break;
        }
        break;
    case 1:
        switch (g_casino_timer) {
        case 1:
            CasinoTween(&D_80095B28, 8, 8, -0x10, -0x10, 8);
            CasinoTween(&D_80095CC0, 8, 8, -0x10, -0x10, 8);
            break;
        case 8:
            CasinoSpritesSetOn(0x277, 8, 0);
            CasinoSpritesSetOn(0x28A, 0xB, 0);
            CasinoTween(&D_80095BF4, 0, -0x18, 0, 0x20, 8);
            CasinoPlaySeq(&g_casino_seqs[3], (u_long *)0x139B0C, g_casino_vab);
            break;
        case 0x10:
            CasinoTween(&D_80095BF4, 0, -0x20, 0, -0x20, 0x10);
            break;
        }
        break;
    case 2:
        switch (g_casino_timer) {
        case 1:
            CasinoTween(&D_80095B28, 8, 8, -0x10, -0x10, 8);
            CasinoTween(&D_80095BF4, 8, 8, -0x10, -0x10, 8);
            break;
        case 8:
            CasinoSpritesSetOn(0x277, 8, 0);
            CasinoSpritesSetOn(0x27F, 0xB, 0);
            CasinoTween(&D_80095CC0, 0, -0x18, 0, 0x20, 8);
            CasinoPlaySeq(&g_casino_seqs[3], (u_long *)0x139C38, g_casino_vab);
            break;
        case 0x10:
            CasinoTween(&D_80095CC0, 0, -0x30, 0, -0x20, 0x10);
            break;
        }
        break;
    }
}

void CasinoPokerShowCard(u_char i)
{
    short face;

    face = func_80082454(g_poker_hand[i]);
    func_800824CC(i, g_poker_hand[i], face, i * 0x15 + 0x8D);
    func_800825A4(g_poker_hand[i], i * 0x15 + 0x8D);
}

void CasinoPokerCycleCard(u_char i, u_char up)
{
    short face;

    SsSeqStop(g_casino_seqs[4]);
    SsSeqPlay(g_casino_seqs[4], 1, 1);
    if (up) {
        if ((s8)++g_poker_hand[i] >= 0x35) {
            g_poker_hand[i] = 0;
        }
    } else {
        if ((s8)--g_poker_hand[i] < 0) {
            g_poker_hand[i] = 0x34;
        }
    }
    face = func_80082454(g_poker_hand[i]);
    func_800824CC(i, g_poker_hand[i], face, i * 0x15 + 0x8D);
    func_800825A4(g_poker_hand[i], i * 0x15 + 0x8D);
    CasinoSetXform(&g_casino_xforms[i]);
    CasinoDrawObj(&g_casino_objs[i]);
}

void CasinoPokerHud(void)
{
    if (g_casino_bet != g_casino_bet_shown && g_casino_timer != 0) {
        if (!g_casino_bet_shown) {
            if (g_casino_bet == 1) {
                g_casino_lamps |= 0x14;
            }
        } else if (!g_casino_bet) {
            g_casino_lamps &= ~0x10;
            g_casino_lamps &= ~4;
        }
        if (g_casino_bet < 6) {
            CasinoPokerLightRow(g_casino_bet, g_casino_bet_shown);
        }
        if (g_casino_bet == 0 || g_casino_bet >= 6) {
            CasinoPokerBetDigits(g_casino_bet, 2);
            CasinoPokerPayTable(g_casino_bet, 2);
        }
        g_casino_bet_shown = g_casino_bet;
    }
    if (g_casino_money != g_casino_money_shown) {
        CasinoShowNumber(g_casino_money, g_poker_money_digits, 8);
        g_casino_money_shown = g_casino_money;
    }
    if (g_poker_jackpot != g_casino_jackpot_shown) {
        CasinoShowCents(g_poker_jackpot, g_poker_jackpot_digits, 8);
        g_casino_jackpot_shown = g_poker_jackpot;
    }
    if (g_casino_jackpot_add && !(g_casino_frame & 1)) {
        if (g_poker_jackpot <= 99999998) {
            g_poker_jackpot++;
        }
        g_casino_jackpot_add--;
    }
    if (CASINO_LAMPS.b0 != CASINO_LAMPS.b1) {
        CasinoStartAnim(D_800953E0, 0x10, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
        CASINO_LAMPS.b1 = CASINO_LAMPS.b0;
    }
    if (CASINO_LAMPS.b2 != CASINO_LAMPS.b3) {
        CasinoStartAnim(D_80095470, 0x10, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
        CASINO_LAMPS.b3 = CASINO_LAMPS.b2;
    }
    if (CASINO_LAMPS.b4 != CASINO_LAMPS.b5) {
        CasinoStartAnim(D_800955C4, 0x20, CASINO_LAMPS.b4 ? 0x800 : -0x800, 0, 0, 0, 0, 0, 0, 0, 0);
        CASINO_LAMPS.b5 = CASINO_LAMPS.b4;
    }
}

void CasinoPokerPlaceBet(void)
{
    CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139AE4, g_casino_vab);
    g_casino_step = POKER_START_HAND;
    g_casino_timer = -1;
    g_casino_jackpot_add = g_casino_bet;
}

void CasinoPokerQuit(void)
{
    CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139EF8, g_casino_main_vab);
    if (g_casino_bet) {
        SsSeqStop(g_casino_seqs[5]);
        SsSeqPlay(g_casino_seqs[5], 1, 1);
        g_casino_money += g_casino_bet;
        g_casino_bet = 0;
        if (g_casino_bet < 6) {
            CasinoPokerLightRow(g_casino_bet, g_casino_bet_shown);
        }
        if (g_casino_bet == 0 || g_casino_bet >= 6) {
            CasinoPokerBetDigits(g_casino_bet, 2);
            CasinoPokerPayTable(g_casino_bet, 2);
        }
    }
    g_casino_game = CASINO_GAME_LEAVE;
    CASINO_LAMPS.b0 = 0;
    CASINO_LAMPS.b2 = 0;
    CASINO_LAMPS.b4 = 0;
    CasinoFade(0, 0x2F6, 0, 0, 0, 0x20);
    g_casino_timer = -1;
}

void CasinoPokerMoveCursor(void)
{
    CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
    CasinoStartAnim(&g_casino_objs[g_casino_cursor.x], 8, 0, 0, 0, 0, -8, 0, 0, 0, 0);
    CasinoTween(&D_800957E4[g_casino_cursor.x], 0, -8, 0, 0, 8);
    CasinoStartAnim(&g_casino_objs[g_casino_cursor.prev_x], 8, 0, 0, 0, 0, 8, 0, 0, 0, 0);
    CasinoTween(&D_800957E4[g_casino_cursor.prev_x], 0, 8, 0, 0, 8);
}

void CasinoPokerHoldKeys(void)
{
    if ((g_casino_pad_trig & PAD_CIRCLE) || (g_casino_pad_trig & PAD_SQUARE) || (g_casino_pad_trig & PAD_TRIANGLE) ||
        (g_casino_pad_trig & PAD_CROSS)) {
        if (!g_poker_held[g_casino_cursor.x]) {
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            g_poker_held[g_casino_cursor.x] = 1;
            CasinoTween(&D_800957E4[g_casino_cursor.x], -0x10, 0, 0x20, 8, 8);
        } else if (g_poker_held[g_casino_cursor.x] == 1) {
            SsSeqStop(g_casino_seqs[5]);
            SsSeqPlay(g_casino_seqs[5], 1, 1);
            g_poker_held[g_casino_cursor.x] = 0;
            CasinoTween(&D_800957E4[g_casino_cursor.x], 0x10, 0, -0x20, -8, 8);
        }
    }
}

void CasinoPokerDummy(void)
{
}

void CasinoPokerStartDraw(void)
{
    int i;

    SsSeqStop(g_casino_seqs[4]);
    SsSeqPlay(g_casino_seqs[4], 1, 1);
    g_casino_step = POKER_JUDGE;
    CasinoStartAnim(&g_casino_objs[g_casino_cursor.x], 8, 0, 0, 0, 0, 8, 0, 0, 0, 0);
    if (g_poker_held[g_casino_cursor.x] == 1) {
        CasinoTween(&D_800957E4[g_casino_cursor.x], 0, 8, 0, 0, 8);
    }
    for (i = 0; i < POKER_CARDS; i++) {
        if (!g_poker_held[i]) {
            CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139E94, g_casino_main_vab);
            CasinoStartAnim(&g_casino_objs[i], 0x20, 0, 0, 0, -((10 - i) * 32), 0, 0, 0, 0, 0);
            g_casino_step = POKER_DRAW;
        }
        g_poker_flipped[i] = g_poker_held[i];
    }
    CasinoQueueCluts(0x102, 0xA, 0x7CA4);
    g_casino_timer = -1;
    CASINO_LAMPS.b2 = 0;
}

void CasinoPokerLightRow(bet, shown)
short bet;
short shown;
{
    short row;
    short prev;

    row = bet < 5 ? bet - 1 : 4;
    prev = shown < 5 ? shown - 1 : 4;
    if (row != prev) {
        if (shown) {
            CasinoQueueCluts(prev * 10 + 0x11D, 10, 0x7CA4);
        }
        if (bet) {
            CasinoQueueCluts(row * 10 + 0x11D, 10, 0x7CE4);
        }
    }
}

/* Paints the pay table for a bet: five rows of multipliers from k, each
   rank's payout in four digits with leading zeroes blank. At the top
   bet the last row's first entry reads as text instead. */
void CasinoPokerPayTable(bet)
short bet;
{
    short d[4];
    short k;
    short pay;
    int   row;
    int   rank;
    int   j;
    u_char lit;

    k = bet - 4;
    if (k < 0) {
        k = 1;
    }
    for (row = 0; row < 5; row++) {
        for (rank = 0; rank < POKER_RANKS; rank++) {
            pay = g_poker_pay[rank] * (k + row);
            d[0] = pay % 10000 / 1000;
            d[1] = pay % 1000 / 100;
            d[2] = pay % 100 / 10;
            d[3] = pay % 10;
            lit = 0;
            for (j = 0; j < 4; j++) {
                if (d[j] == 0 && !lit) {
                    D_80094E28[row].cells[rank * 4 + j].on = 0;
                    d[j] = 0xE;
                    CasinoSpritesSetOn(row * 0x24 + 0x154 + rank * 4 + j, 1, 0);
                } else {
                    D_80094E28[row].cells[rank * 4 + j].on = 1;
                    lit = 1;
                    CasinoSpritesSetOn(row * 0x24 + 0x154 + rank * 4 + j, 1, 1);
                }
                D_80094E28[row].tex_idx[rank * 4 + j] = d[j];
            }
            if (k + 4 == 10 && row == 4 && rank == 0) {
                D_80094E28[4].tex_idx[0] = 0xE;
                D_80094E28[4].tex_idx[1] = 0xB;
                D_80094E28[4].tex_idx[2] = 0xC;
                D_80094E28[4].tex_idx[3] = 0xD;
            }
        }
        CasinoTexLayout(&D_80094E28[row]);
    }
}

/* Called with one argument and with two; defined old-style. */
void CasinoPokerBetDigits(bet)
short bet;
{
    short k;

    if (bet < 6) {
        k = 1;
    } else {
        k = bet - 4;
    }
    D_80094D9C.tex_idx[0] = k;
    D_80094D9C.tex_idx[1] = k + 1;
    D_80094D9C.tex_idx[2] = k + 2;
    D_80094D9C.tex_idx[3] = k + 3;
    D_80094D9C.tex_idx[4] = k + 4;
    CasinoTexLayout(&D_80094D9C);
}
