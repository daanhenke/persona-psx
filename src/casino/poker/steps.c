/* Persona 1 (JP) - CASINO's video poker: the step machine, from the table
 * opening to the win.
 *   0x8006D25C CasinoPokerRun
 *   0x8006D3B8 CasinoPokerOpen
 *   0x8006D460 CasinoPokerEnter
 *   0x8006D62C CasinoPokerBet
 *   0x8006D7C4 CasinoPokerStartHand
 *   0x8006D8C8 CasinoPokerDeal
 *   0x8006DA28 CasinoPokerHold
 *   0x8006DBAC CasinoPokerDraw
 *   0x8006DDA0 CasinoPokerJudge
 *   0x8006DF4C CasinoPokerWin
 *
 * Each step runs once a frame; g_casino_timer counts the frames since it
 * began, and a step hands over by setting the next one and the timer to
 * -1 (the frame wrap makes that 0).
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>
#include <persona/casino/casino.h>
#include <persona/casino/poker.h>

extern void CasinoGame1LoadVram(void);
extern void CasinoGame1LoadSound(void);
extern void CasinoFade(short first, short count, u_char r, u_char g, u_char b, short frames);
extern void CasinoStartPalAnim(CasinoPalAnim *a);
extern void CasinoQueueCluts(); /* (short first, short n, int clut), called unprototyped */
extern void CasinoSpritesSetOn(short first, short n, int on);
extern void CasinoShowLayout(CasinoLayout *l, u_char mode);
extern void CasinoTween(CasinoLayout *l, short dx, short dy, short dw, short dh, short frames);
extern void CasinoCursorClear(CasinoCursor *c);
extern void CasinoCursorRepeat(CasinoCursor *c, u_char mode, short rate);
extern void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                            long sy, long sz);
extern void CasinoPlaySeq(short *h, u_long *seq, short vab);

extern void   func_8006E050(void);
extern void   func_8006E544(void);
extern void   func_8006E88C(void);
extern void   func_8006EEDC(void);
extern void   func_8006F018(void);
extern void   func_8006F294(void);
extern void   func_8006FC18(void);
extern void   func_8006FCBC(void);
extern void   func_80070890(void);
extern void   func_800709A8(void);
extern void   func_80070A18(void);
extern void   func_80070D64(void);
extern void   func_80070E98(void);
extern void   func_80070F58(void);
extern void   func_800715C8(void);
extern void   func_800718EC(void);
extern void   func_80071948(void);
extern void   func_80071A7C(void);
extern void   func_80071BB8(void);
extern void   func_80071D04(void);
extern u_char func_800723A4(u_char *hand, u_char *marks);
extern void   func_80072AF0(u_char rank);
extern int    func_80072C34(u_char rank);
extern void   func_80072D14(void);

extern s8 g_casino_key_fire[4];
extern s8 g_casino_key_hold[4];

/* The table's pieces. */
extern CasinoLayout    D_800954C4;
extern CasinoLayoutDef D_80095630;
extern CasinoLayoutDef D_80095688;
extern CasinoLayout    D_80095A90;
extern CasinoLayout    D_80095B28;
extern CasinoLayout    D_80095BF4;
extern CasinoLayout    D_80095CC0;

void CasinoPokerOpen(void);
void CasinoPokerEnter(void);
void CasinoPokerBet(void);
void CasinoPokerStartHand(void);
void CasinoPokerDeal(void);
void CasinoPokerHold(void);
void CasinoPokerDraw(void);
void CasinoPokerJudge(void);
void CasinoPokerWin(void);

void CasinoPokerRun(void)
{
    switch (g_casino_step) {
    case POKER_OPEN:
        CasinoPokerOpen();
        break;
    case POKER_ENTER:
        CasinoPokerEnter();
        break;
    case POKER_BET:
        CasinoPokerBet();
        break;
    case POKER_START_HAND:
        CasinoPokerStartHand();
        break;
    case POKER_DEAL:
        CasinoPokerDeal();
        break;
    case POKER_HOLD:
        CasinoPokerHold();
        break;
    case POKER_DRAW:
        CasinoPokerDraw();
        break;
    case POKER_JUDGE:
        CasinoPokerJudge();
        break;
    case POKER_WIN:
        CasinoPokerWin();
        break;
    case POKER_DOUBLE_UP:
        func_8006E050();
        break;
    case 0x70:
        func_8006E544();
        break;
    case POKER_LOSE:
        func_8006EEDC();
        break;
    case 0x30:
        func_8006F018();
        break;
    case 0x31:
        func_8006E88C();
        break;
    case 0x33:
        func_8006F294();
        break;
    case 0x1A:
        func_80072D14();
        break;
    }
    if (g_casino_step != POKER_OPEN) {
        func_800715C8();
    }
}

void CasinoPokerOpen(void)
{
    if (g_casino_timer == 0) {
        CasinoGame1LoadVram();
    }
    if (g_casino_timer == 1) {
        CasinoGame1LoadSound();
        func_8006FC18();
        func_8006FCBC();
        CasinoFade(0, 0x2F6, 0x80, 0x80, 0x80, 0x20);
    }
    if (g_casino_timer > 0x20) {
        g_casino_timer = -1;
        g_casino_step = POKER_ENTER;
    }
}

void CasinoPokerEnter(void)
{
    if (!g_poker_palanim0->on) {
        CasinoStartPalAnim(g_poker_palanim0);
    }
    if (g_casino_timer == 0) {
        CasinoQueueCluts(0x24E, 0xE, GetClut(g_poker_palanim0->r.x, g_poker_palanim0->r.y));
        CasinoQueueCluts(0x25C, 8, GetClut(g_poker_palanim0->r.x, g_poker_palanim0->r.y));
        D_800AFC98.b0 = 1;
        func_80070890();
        func_800709A8();
        CasinoCursorClear((CasinoCursor *)g_casino_key_fire);
        CasinoCursorClear((CasinoCursor *)g_casino_key_hold);
        CasinoShowLayout(&D_80095A90, 0);
        CasinoShowLayout(&D_80095B28, 0);
        CasinoShowLayout(&D_80095BF4, 0);
        CasinoShowLayout(&D_80095CC0, 0);
        CasinoSpritesSetOn(0xF5, 0x5F, 1);
        CasinoSpritesSetOn(0x233, 0x31, 1);
        CasinoSpritesSetOn(0x14, 0x1E, 0);
        CasinoTween(D_80095630.l, 0x140, 0, 0, 0, 0x20);
        CasinoTween(D_80095688.l, -0x140, 0, 0, 0, 0x20);
    }
    if (g_casino_timer == 0x20) {
        CasinoSpritesSetOn(0x24E, 0xE, 1);
        g_casino_timer = -1;
        g_casino_step = POKER_BET;
    }
}

void CasinoPokerBet(void)
{
    if (!g_poker_palanim0->on) {
        CasinoStartPalAnim(g_poker_palanim0);
    }
    if (!g_casino_max_bet) {
        func_80070A18();
    } else if (g_casino_money) {
        if (g_casino_bet % 3 == 0) {
            SsSeqStop(g_casino_seqs[6]);
        }
        SsSeqPlay(g_casino_seqs[6], 1, 1);
        g_casino_bet++;
        g_casino_money--;
    } else {
        g_casino_max_bet = 0;
        g_casino_bet_done = 1;
    }
    if (g_casino_bet >= 10 || (g_casino_bet != 0 && g_casino_money == 0)) {
        g_casino_bet_done = 1;
        g_casino_max_bet = 0;
    }
    if (g_casino_bet_done == 1 && g_casino_timer > 0) {
        func_800718EC();
    }
    if (g_casino_quit == 1) {
        func_80071948();
    }
}

void CasinoPokerStartHand(void)
{
    if (g_casino_timer == 0) {
        CasinoTween(D_80095630.l, -0x140, 0, 0, 0, 0x20);
        CasinoTween(D_80095688.l, 0x140, 0, 0, 0, 0x20);
        CasinoTween(&D_800954C4, -0x10, 8, 0x20, -8, 8);
        D_800AFC98.b4 = 0;
        D_800AFC98.b0 = 0;
    }
    if (g_casino_timer > 8) {
        CasinoQueueCluts(0x24E, 0xE, 0x7C24);
        CasinoQueueCluts(0x25C, 8, 0x7C24);
        g_casino_timer = -1;
        g_casino_step = POKER_DEAL;
    }
}

void CasinoPokerDeal(void)
{
    if (g_casino_timer == 0) {
        func_80070D64();
        g_poker_rank = func_800723A4(g_poker_hand, g_poker_marks);
    }
    if (g_casino_timer == 0x18) {
        CasinoSpritesSetOn(0x233, 0x31, 0);
    }
    if (g_casino_timer <= 0x28 && g_casino_timer % 10 == 0) {
        CasinoStartAnim(&g_casino_objs[g_casino_timer / 10], 0x30, 0, -0x1F80, 0, -(g_casino_timer / 10 * 64) + 0x80,
                        0x30, 0, 0x1000, 0x1000, 0x1000);
    }
    if (g_casino_timer > 0x5A) {
        CasinoStartAnim(&g_casino_objs[0], 8, 0, 0, 0, 0, -8, 0, 0, 0, 0);
        g_casino_step = POKER_HOLD;
        g_casino_timer = -1;
    }
}

void CasinoPokerHold(void)
{
    CasinoCursorRepeat(&g_casino_cursor, 1, 10);
    if (g_casino_timer == 0) {
        g_poker_rank = func_800723A4(g_poker_hand, g_poker_marks);
        if (g_poker_rank) {
            func_80070F58();
            CasinoQueueCluts(g_poker_hand_spr[g_poker_rank - 1], 1, GetClut(0x390, 0x1F1));
        }
        CasinoSpritesSetOn(0x3C, 0x1E, 0);
        g_casino_cursor.x = 0;
        g_casino_cursor.prev_x = 0;
    }
    if (g_poker_rank) {
        if (!g_poker_palanim1->on) {
            CasinoStartPalAnim(g_poker_palanim1);
        }
        if (!g_poker_palanim2->on) {
            CasinoStartPalAnim(g_poker_palanim2);
        }
    }
    if (g_casino_cursor.x != g_casino_cursor.prev_x) {
        func_80071A7C();
    }
    func_80071BB8();
    if ((g_casino_pad_trig & 8) || (g_casino_pad_trig & 2)) {
        func_80071D04();
    }
}

void CasinoPokerDraw(void)
{
    int i;

    if (g_casino_timer == 0) {
        CasinoSpritesSetOn(0x3C, 0x1E, 1);
        g_poker_flip_idx = 0;
        for (i = 0; i < POKER_CARDS; i++) {
            CasinoQueueCluts(g_poker_card_spr[i] + 0x12, 2, 0x7C25);
        }
    }
    if (g_casino_timer == 0x28) {
        func_80070E98();
    }
    if (g_casino_timer > 0x28 && g_poker_flip_idx < POKER_CARDS) {
        if (!g_poker_flipped[g_poker_flip_idx]) {
            CasinoStartAnim(&g_casino_objs[g_poker_flip_idx], 0x20, 0, 0, 0, (10 - g_poker_flip_idx) * 32, 0, 0, 0, 0,
                            0);
            g_poker_flipped[g_poker_flip_idx] = 1;
            g_poker_flip_sound = 1;
        } else if (!g_casino_objs[g_poker_flip_idx].busy) {
            if (g_poker_flip_sound) {
                CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
                g_poker_flip_sound = 0;
            }
            g_poker_flip_idx++;
        }
    }
    if (g_poker_flip_idx >= POKER_CARDS) {
        g_casino_timer = -1;
        g_casino_step = POKER_JUDGE;
        CasinoSpritesSetOn(0x3C, 0x1E, 0);
    }
}

void CasinoPokerJudge(void)
{
    short row;
    int   first;

    if (g_casino_timer == 0) {
        g_poker_rank = func_800723A4(g_poker_hand, g_poker_marks);
        if (g_casino_bet < 5) {
            row = g_casino_bet - 1;
        } else {
            row = 4;
        }
        if (g_poker_rank) {
            CasinoQueueCluts(g_poker_hand_spr[g_poker_rank - 1], 1, GetClut(0x390, 0x1F1));
            first = row * 10 + 0x11D;
            CasinoQueueCluts(first, 10, 0x7CA4);
            CasinoQueueCluts(g_poker_rank + first, 1, GetClut(0x390, 0x1F1));
            if (g_poker_rank == 1 && g_casino_bet == 10) {
                g_poker_jackpot_hit = 1;
                g_poker_payout = g_casino_win = g_poker_jackpot / 100;
            } else {
                g_poker_payout = g_casino_win = g_poker_pay[g_poker_rank - 1] * g_casino_bet;
            }
            g_casino_step = POKER_WIN;
        } else {
            g_casino_step = POKER_LOSE;
        }
        g_casino_timer = -1;
    }
}

void CasinoPokerWin(void)
{
    if (!g_poker_palanim1->on) {
        CasinoStartPalAnim(g_poker_palanim1);
    }
    if (!g_poker_palanim2->on) {
        CasinoStartPalAnim(g_poker_palanim2);
    }
    if (g_casino_timer == 0) {
        func_80070F58();
        g_poker_win_seq = func_80072C34(g_poker_rank);
        SsSeqSetVol(g_casino_seqs[15], 0x20, 0x20);
    }
    if (g_poker_win_seq != -1 && !SsIsEos(g_casino_seqs[0], 0)) {
        func_80072AF0(g_poker_rank);
        g_poker_win_seq = -1;
        g_casino_timer = -1;
        g_casino_step = POKER_DOUBLE_UP;
    }
}
