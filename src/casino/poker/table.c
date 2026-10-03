/* Persona 1 (JP) - CASINO's video poker: setting the table up, the money
 * and jackpot readouts, the bet keys, and dealing and drawing cards.
 *   0x8006FC18 CasinoPokerInit
 *   0x8006FCBC CasinoPokerBuild
 *   0x800701AC CasinoShowNumber
 *   0x8007051C CasinoShowCents
 *   0x80070890 CasinoPokerNewHand
 *   0x800709A8 CasinoPokerCursorInit
 *   0x80070A18 CasinoPokerBetKeys
 *   0x80070C68 CasinoPokerBetLamps
 *   0x80070CF8 CasinoPokerShowBet
 *   0x80070D64 CasinoPokerDealCards
 *   0x80070E98 CasinoPokerDrawCards
 *   0x80070F58 CasinoPokerLightMarks
 *   0x80070FDC CasinoTurnObj
 *
 * A readout is a row of one-sprite layouts, the lowest digit first; each
 * digit is a scroll of its texture by eight pixels a glyph, glyph 11 the
 * blank that stands in for leading zeroes.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>
#include <persona/casino/casino.h>
#include <persona/casino/poker.h>

extern void CasinoQueueCluts(); /* (short first, short n, int clut), called unprototyped */
extern void CasinoScroll();     /* (CasinoLayout *l, short du, short dv, short dw, short dh, short frames) */
extern void CasinoShowLayout(CasinoLayout *l, u_char mode);
extern void CasinoBuildLayouts(CasinoLayoutDef *d, int mode);
extern void CasinoBuildFrame(CasinoFrame *d);
extern void CasinoBuildObj(CasinoModel *m);
extern void CasinoTexObj(CasinoModel *m);
extern void CasinoSetXform(CasinoXform *x);
extern void CasinoDrawObj(CasinoObj *o);
extern void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                            long sy, long sz);
extern void CasinoPlaySeq(short *h, u_long *seq, short vab);

extern void CasinoPokerLightRow(); /* (short bet, short shown), defined old-style */
extern void CasinoPokerPayTable(); /* (u_char bet, u_char mode); one caller passes only the bet */
extern void CasinoPokerBetDigits(); /* the same */
extern void CasinoPokerShowCard(u_char i);
extern void func_80082658(u_char *out, int n, int range, int unique);
extern int  func_80082454(u_char card);
extern void func_800824CC(int i, u_char card, int face, int spr);
extern void func_800825A4(u_char card, int spr);

extern u_char      D_800B0AF8[4];
extern int         g_casino_money_shown;
extern CasinoXform g_casino_xforms[25];

void CasinoShowNumber(u_int n, CasinoLayout *digits, u_char count);
void CasinoShowCents(u_int n, CasinoLayout *digits, u_char count);

void CasinoPokerInit(void)
{
    D_800B0AF8[1] = 0;
    D_800B0AF8[2] = 0;
    g_casino_money_shown = g_casino_money;
    g_casino_max_bet = 0;
    g_casino_bet_shown = 0;
    g_casino_bet = 0;
    g_casino_quit = 0;
    g_poker_jackpot_add = 0;
    g_poker_jackpot_hit = 0;
    g_poker_jackpot_shown = g_poker_jackpot;
    CASINO_LAMPS.b5 = 0;
    CASINO_LAMPS.b4 = 0;
    CASINO_LAMPS.b3 = 0;
    CASINO_LAMPS.b2 = 0;
    CASINO_LAMPS.b1 = 0;
    CASINO_LAMPS.b0 = 0;
}

void CasinoPokerBuild(void)
{
    CasinoBuildLayouts(D_8009479C, 0);
    CasinoBuildLayouts(D_8009485C, 0);
    CasinoBuildFrame(&D_800948A4);
    CasinoBuildLayouts(D_80094958, 0);
    CasinoBuildLayouts(D_80094A20, 0);
    CasinoBuildFrame(&D_80094A60);
    CasinoBuildLayouts(D_80094B78, 0);
    CasinoBuildLayouts(D_80094C98, 0);
    CasinoBuildLayouts(D_80094DB4, 1);
    CasinoPokerBetDigits(0);
    CasinoBuildLayouts(D_80094EA0, 1);
    CasinoPokerPayTable(0);
    CasinoShowLayout(&D_8009520C, 0);
    CasinoBuildFrame(&D_800951E4);
    CasinoShowLayout(&D_80095164, 0);
    CasinoShowLayout(&g_poker_jackpot_digits[0], 0);
    CasinoShowLayout(&g_poker_jackpot_digits[1], 0);
    CasinoShowLayout(&g_poker_jackpot_digits[2], 0);
    CasinoShowLayout(&g_poker_jackpot_digits[3], 0);
    CasinoShowLayout(&g_poker_jackpot_digits[4], 0);
    CasinoShowLayout(&g_poker_jackpot_digits[5], 0);
    CasinoShowLayout(&g_poker_jackpot_digits[6], 0);
    CasinoShowLayout(&g_poker_jackpot_digits[7], 0);
    CasinoShowLayout(&g_poker_jackpot_digits[8], 0);
    CasinoShowLayout(&D_8009527C, 0);
    CasinoBuildFrame(&D_80095254);
    CasinoShowLayout(&D_8009519C, 0);
    CasinoShowLayout(&g_poker_money_digits[0], 0);
    CasinoShowLayout(&g_poker_money_digits[1], 0);
    CasinoShowLayout(&g_poker_money_digits[2], 0);
    CasinoShowLayout(&g_poker_money_digits[3], 0);
    CasinoShowLayout(&g_poker_money_digits[4], 0);
    CasinoShowLayout(&g_poker_money_digits[5], 0);
    CasinoShowLayout(&g_poker_money_digits[6], 0);
    CasinoShowLayout(&g_poker_money_digits[7], 0);
    CasinoShowLayout(&D_800952EC, 0);
    CasinoBuildFrame(&D_800952C4);
    CasinoBuildObj(&D_800953F0);
    CasinoTexObj(&D_800953F0);
    CasinoSetXform(D_800953E0->xform);
    CasinoDrawObj(D_800953E0);
    CasinoShowLayout(&D_8009535C, 0);
    CasinoBuildFrame(&D_80095334);
    CasinoBuildObj(&D_80095480);
    CasinoTexObj(&D_80095480);
    CasinoSetXform(D_80095470->xform);
    CasinoDrawObj(D_80095470);
    CasinoBuildLayouts(D_800954DC, 0);
    CasinoBuildObj(&D_800955D4);
    CasinoTexObj(&D_800955D4);
    CasinoSetXform(D_800955C4->xform);
    CasinoDrawObj(D_800955C4);
    CasinoBuildLayouts(D_80095630, 0);
    CasinoBuildLayouts(D_80095688, 0);
    CasinoShowLayout(&D_800957E4[0], 0);
    CasinoShowLayout(&D_800957E4[1], 0);
    CasinoShowLayout(&D_800957E4[2], 0);
    CasinoShowLayout(&D_800957E4[3], 0);
    CasinoShowLayout(&D_800957E4[4], 0);
    CasinoBuildObj(&D_80095780);
    CasinoTexObj(&D_80095780);
    CasinoSetXform(D_80095770->xform);
    CasinoDrawObj(D_80095770);
    CasinoBuildObj(&D_800962F4);
    CasinoTexObj(&D_800962F4);
    CasinoSetXform(D_800962E4->xform);
    CasinoDrawObj(D_800962E4);
    CasinoBuildObj(&D_80096250);
    CasinoTexObj(&D_80096250);
    CasinoSetXform(D_80096240->xform);
    CasinoDrawObj(D_80096240);
    CasinoBuildObj(&D_80096398);
    CasinoTexObj(&D_80096398);
    CasinoSetXform(D_80096388->xform);
    CasinoDrawObj(D_80096388);
    CasinoBuildObj(&D_80096494);
    CasinoTexObj(&D_80096494);
    CasinoSetXform(D_80096484->xform);
    CasinoDrawObj(D_80096484);
    CasinoQueueCluts(0x2B7, 4, GetClut(0x390, 0x1F1));
    CasinoQueueCluts(0x2BF, 8, GetClut(0x390, 0x1F1));
    CasinoShowNumber(g_casino_money, g_poker_money_digits, 8);
    CasinoShowCents(g_poker_jackpot, g_poker_jackpot_digits, 8);
}

void CasinoShowNumber(u_int n, CasinoLayout *digits, u_char count)
{
    int d[8];
    int i;
    int blank;

    d[7] = n % 100000000 / 10000000;
    d[6] = n % 10000000 / 1000000;
    d[5] = n % 1000000 / 100000;
    d[4] = n % 100000 / 10000;
    d[3] = n % 10000 / 1000;
    d[2] = n % 1000 / 100;
    d[1] = n % 100 / 10;
    d[0] = n % 10;
    blank = 1;
    for (i = count - 1; i >= 0; i--) {
        g_casino_texs[digits[i].first].u = 0;
        if (d[i]) {
            blank = 0;
        }
        if (!d[i] && i != 0) {
            if (blank == 1) {
                CasinoScroll(&digits[i], 0x58, 0, 0, 0, 1);
            } else {
                CasinoScroll(&digits[i], 0, 0, 0, 0, 1);
            }
        } else {
            CasinoScroll(&digits[i], d[i] * 8, 0, 0, 0, 1);
        }
    }
}

void CasinoShowCents(u_int n, CasinoLayout *digits, u_char count)
{
    int d[9];
    int i;
    int blank;

    d[8] = n % 100000000 / 10000000;
    d[7] = n % 10000000 / 1000000;
    d[6] = n % 1000000 / 100000;
    d[5] = n % 100000 / 10000;
    d[4] = n % 10000 / 1000;
    d[3] = n % 1000 / 100;
    d[2] = 0xE;
    d[1] = n % 100 / 10;
    d[0] = n % 10;
    blank = 1;
    for (i = count; i >= 0; i--) {
        g_casino_texs[digits[i].first].u = 0;
        if (d[i]) {
            blank = 0;
        }
        if (!d[i] && i != 0) {
            if (blank == 1) {
                CasinoScroll(&digits[i], 0x58, 0, 0, 0, 1);
            } else {
                CasinoScroll(&digits[i], 0, 0, 0, 0, 1);
            }
        } else {
            CasinoScroll(&digits[i], d[i] * 8, 0, 0, 0, 1);
        }
    }
}

void CasinoPokerNewHand(void)
{
    g_poker_hand[0] = 0;
    g_poker_deck[0] = 0;
    g_poker_hand[1] = 0;
    g_poker_deck[1] = 0;
    g_poker_hand[2] = 0;
    g_poker_deck[2] = 0;
    g_poker_hand[3] = 0;
    g_poker_deck[3] = 0;
    g_poker_hand[4] = 0;
    g_poker_deck[4] = 0;
    g_poker_deck[5] = 0;
    g_poker_deck[6] = 0;
    g_poker_deck[7] = 0;
    g_poker_deck[8] = 0;
    g_poker_deck[9] = 0;
    g_poker_held[0] = 0;
    g_poker_held[1] = 0;
    g_poker_held[2] = 0;
    g_poker_held[3] = 0;
    g_poker_held[4] = 0;
    g_casino_win_digits[0] = 0;
    g_casino_win_digits[1] = 0;
    g_casino_win_digits[2] = 0;
    g_casino_win_digits[3] = 0;
    g_casino_win_digits[4] = 0;
    g_casino_win_digits[5] = 0;
    g_casino_win_digits[6] = 0;
    g_casino_win_digits[7] = 0;
    g_casino_bet_shown = g_casino_bet;
    g_casino_bet = 0;
    g_casino_win = 0;
    g_casino_pay_digit = 0;
    g_casino_bet_done = 0;
}

void CasinoPokerCursorInit(void)
{
    g_casino_cursor.w = 5;
    g_casino_cursor.h = 5;
    g_casino_cursor.prev_x = 0;
    g_casino_cursor.x = 0;
    g_casino_cursor.prev_y = 0;
    g_casino_cursor.y = 0;
    g_casino_cursor.wrap_x = 1;
    g_casino_cursor.wrap_y = 1;
    g_casino_cursor.unk8 = 0;
    g_casino_cursor.unk9 = 0;
    g_casino_cursor.unkA = 0;
    g_casino_cursor.unkB = 0;
}

void CasinoPokerBetKeys(void)
{
    if (g_casino_pad_trig & PAD_TRIANGLE) {
        if (g_casino_money) {
            if (g_casino_bet < 10) {
                g_casino_max_bet = 1;
            } else {
                g_casino_bet_done = 1;
            }
        } else {
            CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139ED0, g_casino_main_vab);
        }
    } else if (g_casino_pad_trig & PAD_CIRCLE) {
        if (g_casino_bet < 10) {
            if (g_casino_money) {
                SsSeqStop(g_casino_seqs[6]);
                SsSeqPlay(g_casino_seqs[6], 1, 1);
                g_casino_bet++;
                g_casino_money--;
            } else {
                CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139ED0, g_casino_main_vab);
            }
        } else {
            g_casino_bet_done = 1;
        }
    } else if ((g_casino_pad_trig & PAD_CROSS) && !g_casino_max_bet && g_casino_bet) {
        SsSeqStop(g_casino_seqs[5]);
        SsSeqPlay(g_casino_seqs[5], 1, 1);
        g_casino_bet = 0;
        g_casino_money += g_casino_bet_shown;
    } else if (((g_casino_pad_trig & PAD_R1) || (g_casino_pad_trig & PAD_R2)) && !g_casino_max_bet && g_casino_bet) {
        SsSeqStop(g_casino_seqs[4]);
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        g_casino_bet_done = 1;
    }
    if (((g_casino_pad_trig & PAD_L1) || (g_casino_pad_trig & PAD_L2)) && !g_casino_bet_done) {
        g_casino_quit = 1;
    }
}

void CasinoPokerBetLamps(void)
{
    if (!g_casino_bet_shown && g_casino_bet == 1) {
        CASINO_LAMPS.b2 = 1;
        CASINO_LAMPS.b4 = 1;
    }
    if (g_casino_bet_shown && !g_casino_bet) {
        CASINO_LAMPS.b4 = 0;
        CASINO_LAMPS.b2 = 0;
    }
}

void CasinoPokerShowBet(void)
{
    if (g_casino_bet < 6) {
        CasinoPokerLightRow(g_casino_bet, g_casino_bet_shown);
    }
    if (g_casino_bet == 0 || g_casino_bet >= 6) {
        CasinoPokerBetDigits(g_casino_bet, 2);
        CasinoPokerPayTable(g_casino_bet, 2);
    }
}

void CasinoPokerDealCards(void)
{
    int i;

    func_80082658(g_poker_deck, 10, 0x35, 1);
    for (i = 0; i < POKER_CARDS; i++) {
        g_poker_hand[i] = g_poker_deck[i];
        CasinoPokerShowCard(i);
        g_casino_xforms[i].rot.vx = 0;
        g_casino_xforms[i].rot.vy = -0x80;
        g_casino_xforms[i].rot.vz = 0;
        g_casino_xforms[i].trans.vx = (i - 2) * -8;
        g_casino_xforms[i].trans.vy = 0x10;
        g_casino_xforms[i].trans.vz = 0x200;
        g_casino_xforms[i].scale.vx = 0;
        g_casino_xforms[i].scale.vy = 0;
        g_casino_xforms[i].scale.vz = 0;
        CasinoSetXform(&g_casino_xforms[i]);
        CasinoDrawObj(&g_casino_objs[i]);
    }
}

void CasinoPokerDrawCards(void)
{
    int i;
    int k;

    for (i = 0, k = 0; i < POKER_CARDS; i++) {
        g_poker_marks[i] = 0;
        if (!g_poker_held[i]) {
            g_poker_hand[i] = g_poker_deck[POKER_CARDS + k];
            k++;
            func_800824CC(i, g_poker_hand[i], func_80082454(g_poker_hand[i]), i * 0x15 + 0x8D);
            func_800825A4(g_poker_hand[i], i * 0x15 + 0x8D);
        }
    }
}

void CasinoPokerLightMarks(void)
{
    int i;

    for (i = 0; i < POKER_CARDS; i++) {
        if (g_poker_marks[i] == 1) {
            CasinoQueueCluts(g_poker_card_spr[i] + 0x12, 2, GetClut(0x390, 0x1F0));
        }
    }
}

void CasinoTurnObj(u_char face, CasinoObj *o, short frames, short turns)
{
    short rx;

    if (face == 1) {
        rx = turns * 4096 - o->xform->rot.vx;
    } else if (face == 0) {
        rx = turns * 4096 - o->xform->rot.vx + 0x800;
    }
    CasinoStartAnim(o, frames, rx, 0, 0, 0, 0, 0, 0, 0, 0);
}
