/* Persona 1 (JP) - CASINO's video poker: the step machine, from the table
 * opening to the end of a round. The win is counted out a coin at a time
 * by digit; the other games use the same three helpers for it.
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
 *   0x8006E050 CasinoPokerChoose
 *   0x8006E544 CasinoPokerPeek
 *   0x8006E88C CasinoPokerCollect
 *   0x8006EAF0 CasinoSplitDigits
 *   0x8006ED68 CasinoPayStep
 *   0x8006EE60 CasinoPow10
 *   0x8006EEDC CasinoPokerLose
 *   0x8006F018 CasinoPokerClear
 *   0x8006F294 CasinoPokerDoubleUp
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
extern void CasinoTween(); /* (CasinoLayout *l, short dx, short dy, short dw, short dh, short frames) */
extern void CasinoCursorClear(CasinoCursor *c);
extern void CasinoCursorRepeat(CasinoCursor *c, u_char mode, short rate);
extern void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                            long sy, long sz);
extern void CasinoPlaySeq(short *h, u_long *seq, short vab);
extern void CasinoBuildObj(CasinoModel *m);
extern void CasinoTexObj(CasinoModel *m);
extern void CasinoSetXform(CasinoXform *x);
extern void CasinoDrawObj(CasinoObj *o);

extern u_char g_casino_wrapped;

extern void   CasinoPokerInit(void);
extern void   CasinoPokerBuild(void);
extern void   CasinoPokerNewHand(void);
extern void   CasinoPokerCursorInit(void);
extern void   CasinoPokerBetKeys(void);
extern void   CasinoPokerDealCards(void);
extern void   CasinoPokerDrawCards(void);
extern void   CasinoPokerLightMarks(void);
extern void   CasinoPokerDoublePanels(void);
extern void   CasinoPokerHud(void);
extern void   CasinoPokerPlaceBet(void);
extern void   CasinoPokerQuit(void);
extern void   CasinoPokerMoveCursor(void);
extern void   CasinoPokerHoldKeys(void);
extern void   CasinoPokerStartDraw(void);
extern u_char func_800723A4(u_char *hand, u_char *marks);
extern void   func_80072AF0(u_char rank);
extern int    func_80072C34(u_char rank);
extern void   func_80072D14(void);

extern s8 g_casino_key_fire[4];
extern s8 g_casino_key_hold[4];

/* The double-up table's pieces. */
extern CasinoLayout D_8009488C;
extern CasinoLayout D_80094940;
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
extern CasinoLayout D_80095CF8;
extern CasinoLayout D_80095D30;
extern CasinoLayout D_80095D68;
extern CasinoLayout D_80095E14;
extern CasinoLayout D_8009651C;
extern CasinoLayout D_800965E8;
extern CasinoLayout D_80095E94[8];
extern CasinoLayout D_80095FBC[8];
extern CasinoLayout D_800960E4[8];
extern CasinoObj    D_80095DD0[];
extern CasinoModel  D_80095DE0;

void CasinoPokerOpen(void);
void CasinoPokerEnter(void);
void CasinoPokerBet(void);
void CasinoPokerStartHand(void);
void CasinoPokerDeal(void);
void CasinoPokerHold(void);
void CasinoPokerDraw(void);
void CasinoPokerJudge(void);
void CasinoPokerWin(void);
void CasinoPokerChoose(void);
void CasinoPokerPeek(void);
void CasinoPokerCollect(void);
void CasinoSplitDigits(u_int n);
void CasinoPayStep(int *win);
int  CasinoPow10(u_char unused, u_char n);
void CasinoPokerLose(void);
void CasinoPokerClear(void);
void CasinoPokerDoubleUp(void);

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
    case POKER_CHOOSE:
        CasinoPokerChoose();
        break;
    case POKER_PEEK:
        CasinoPokerPeek();
        break;
    case POKER_LOSE:
        CasinoPokerLose();
        break;
    case POKER_CLEAR:
        CasinoPokerClear();
        break;
    case POKER_COLLECT:
        CasinoPokerCollect();
        break;
    case POKER_DOUBLE_UP:
        CasinoPokerDoubleUp();
        break;
    case POKER_DOUBLE_PLAY:
        func_80072D14();
        break;
    }
    if (g_casino_step != POKER_OPEN) {
        CasinoPokerHud();
    }
}

void CasinoPokerOpen(void)
{
    if (g_casino_timer == 0) {
        CasinoGame1LoadVram();
    }
    if (g_casino_timer == 1) {
        CasinoGame1LoadSound();
        CasinoPokerInit();
        CasinoPokerBuild();
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
        CASINO_LAMPS.b0 = 1;
        CasinoPokerNewHand();
        CasinoPokerCursorInit();
        CasinoCursorClear((CasinoCursor *)g_casino_key_fire);
        CasinoCursorClear((CasinoCursor *)g_casino_key_hold);
        CasinoShowLayout(&D_80095A90, 0);
        CasinoShowLayout(&D_80095B28, 0);
        CasinoShowLayout(&D_80095BF4, 0);
        CasinoShowLayout(&D_80095CC0, 0);
        CasinoSpritesSetOn(0xF5, 0x5F, 1);
        CasinoSpritesSetOn(0x233, 0x31, 1);
        CasinoSpritesSetOn(0x14, 0x1E, 0);
        CasinoTween(D_80095630[0].l, 0x140, 0, 0, 0, 0x20);
        CasinoTween(D_80095688[0].l, -0x140, 0, 0, 0, 0x20);
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
        CasinoPokerBetKeys();
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
        CasinoPokerPlaceBet();
    }
    if (g_casino_quit == 1) {
        CasinoPokerQuit();
    }
}

void CasinoPokerStartHand(void)
{
    if (g_casino_timer == 0) {
        CasinoTween(D_80095630[0].l, -0x140, 0, 0, 0, 0x20);
        CasinoTween(D_80095688[0].l, 0x140, 0, 0, 0, 0x20);
        CasinoTween(&D_800954C4, -0x10, 8, 0x20, -8, 8);
        CASINO_LAMPS.b4 = 0;
        CASINO_LAMPS.b0 = 0;
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
        CasinoPokerDealCards();
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
            CasinoPokerLightMarks();
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
        CasinoPokerMoveCursor();
    }
    CasinoPokerHoldKeys();
    if ((g_casino_pad_trig & 8) || (g_casino_pad_trig & 2)) {
        CasinoPokerStartDraw();
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
        CasinoPokerDrawCards();
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
        CasinoPokerLightMarks();
        g_poker_win_seq = func_80072C34(g_poker_rank);
        SsSeqSetVol(g_casino_seqs[15], 0x20, 0x20);
    }
    if (g_poker_win_seq != -1 && !SsIsEos(g_casino_seqs[0], 0)) {
        func_80072AF0(g_poker_rank);
        g_poker_win_seq = -1;
        g_casino_timer = -1;
        g_casino_step = POKER_CHOOSE;
    }
}

void CasinoPokerChoose(void)
{
    short row;
    int   first;

    if (!g_poker_palanim1->on) {
        CasinoStartPalAnim(g_poker_palanim1);
    }
    if (!g_poker_palanim2->on) {
        CasinoStartPalAnim(g_poker_palanim2);
    }
    if (g_casino_timer == 0) {
        SsSeqSetVol(g_casino_seqs[15], 0x7F, 0x7F);
        if (g_casino_wrapped) {
            SsSeqStop(g_casino_seqs[15]);
        }
        SsSeqPlay(g_casino_seqs[15], 1, 1);
        CasinoFade(0x8D, 0x14, 0x40, 0x40, 0x40, 4);
        CasinoFade(0xA2, 0x14, 0x40, 0x40, 0x40, 4);
        CasinoFade(0xB7, 0x14, 0x40, 0x40, 0x40, 4);
        CasinoFade(0xCC, 0x14, 0x40, 0x40, 0x40, 4);
        CasinoFade(0xE1, 0x14, 0x40, 0x40, 0x40, 4);
        CasinoSpritesSetOn(0x26C, 0xB, 1);
        CasinoSpritesSetOn(0x277, 8, 1);
        CasinoSpritesSetOn(0x27F, 0xB, 1);
        CasinoSpritesSetOn(0x28A, 0xB, 1);
        CasinoTween(&D_80095A90, -8, -8, 0x10, 0x10, 8);
        CasinoTween(&D_80095B28, -8, -8, 0x10, 0x10, 8);
        CasinoTween(&D_80095BF4, -8, -8, 0x10, 0x10, 8);
        CasinoTween(&D_80095CC0, -8, -8, 0x10, 0x10, 8);
    }
    if (g_casino_timer > 8) {
        if (g_casino_pad_trig & PAD_CROSS) {
            g_casino_step = POKER_COLLECT;
        } else if (g_casino_pad_trig & PAD_CIRCLE) {
            g_casino_step = POKER_DOUBLE_UP;
            g_poker_double_game = 0;
            g_poker_double_step = 0x11;
        } else if (g_casino_pad_trig & PAD_TRIANGLE) {
            g_casino_step = POKER_DOUBLE_UP;
            g_poker_double_game = 1;
            g_poker_double_step = 0x11;
        } else if (g_casino_pad_trig & PAD_SQUARE) {
            g_casino_step = POKER_DOUBLE_UP;
            g_poker_double_game = 2;
            g_poker_double_step = 0x11;
        } else if ((g_casino_pad_trig & PAD_RIGHT) || (g_casino_pad_trig & PAD_LEFT) || (g_casino_pad_trig & PAD_UP) ||
                   (g_casino_pad_trig & PAD_DOWN) || (g_casino_pad_trig & PAD_L1) || (g_casino_pad_trig & PAD_L2) ||
                   (g_casino_pad_trig & PAD_R1) || (g_casino_pad_trig & PAD_R2)) {
            g_casino_step = POKER_PEEK;
            g_casino_timer = -1;
        }
        if (g_casino_step == POKER_COLLECT || g_casino_step == POKER_DOUBLE_UP) {
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            CasinoFade(0x8D, 0x14, 0x80, 0x80, 0x80, 4);
            CasinoFade(0xA2, 0x14, 0x80, 0x80, 0x80, 4);
            CasinoFade(0xB7, 0x14, 0x80, 0x80, 0x80, 4);
            CasinoFade(0xCC, 0x14, 0x80, 0x80, 0x80, 4);
            CasinoFade(0xE1, 0x14, 0x80, 0x80, 0x80, 4);
            if (g_casino_step != POKER_COLLECT) {
                CasinoQueueCluts(g_poker_hand_spr[g_poker_rank - 1], 1, 0x7CA4);
                if (g_casino_bet < 5) {
                    row = g_casino_bet - 1;
                } else {
                    row = 4;
                }
                CasinoQueueCluts(g_poker_rank + (first = row * 10 + 0x11D), 1, 0x7CA4);
            }
            if (g_poker_jackpot_hit == 1) {
                g_poker_jackpot = 1000000;
                g_poker_jackpot_hit = 0;
            }
            g_casino_timer = -1;
        }
    }
}

void CasinoPokerPeek(void)
{
    u_char held;

    held = 0;
    if ((g_casino_pad & PAD_RIGHT) || (g_casino_pad & PAD_LEFT) || (g_casino_pad & PAD_UP) || (g_casino_pad & PAD_DOWN) ||
        (g_casino_pad & PAD_L1) || (g_casino_pad & PAD_L2) || (g_casino_pad & PAD_R1) || (g_casino_pad & PAD_R2)) {
        held = 1;
    }
    if (!g_poker_palanim1->on) {
        CasinoStartPalAnim(g_poker_palanim1);
    }
    if (!g_poker_palanim2->on) {
        CasinoStartPalAnim(g_poker_palanim2);
    }
    if (g_casino_timer == 0) {
        CasinoTween(&D_80095B28, 8, 8, -0x10, -0x10, 8);
        CasinoTween(&D_80095BF4, 8, 8, -0x10, -0x10, 8);
        CasinoTween(&D_80095CC0, 8, 8, -0x10, -0x10, 8);
        CasinoTween(&D_80095A90, 8, 8, -0x10, -0x10, 8);
        CasinoFade(0x8D, 0x14, 0x80, 0x80, 0x80, 4);
        CasinoFade(0xA2, 0x14, 0x80, 0x80, 0x80, 4);
        CasinoFade(0xB7, 0x14, 0x80, 0x80, 0x80, 4);
        CasinoFade(0xCC, 0x14, 0x80, 0x80, 0x80, 4);
        CasinoFade(0xE1, 0x14, 0x80, 0x80, 0x80, 4);
    }
    if (g_casino_timer > 8 && !held) {
        CasinoTween(&D_80095B28, -8, -8, 0x10, 0x10, 8);
        CasinoTween(&D_80095BF4, -8, -8, 0x10, 0x10, 8);
        CasinoTween(&D_80095CC0, -8, -8, 0x10, 0x10, 8);
        CasinoTween(&D_80095A90, -8, -8, 0x10, 0x10, 8);
        CasinoFade(0x8D, 0x14, 0x40, 0x40, 0x40, 4);
        CasinoFade(0xA2, 0x14, 0x40, 0x40, 0x40, 4);
        CasinoFade(0xB7, 0x14, 0x40, 0x40, 0x40, 4);
        CasinoFade(0xCC, 0x14, 0x40, 0x40, 0x40, 4);
        CasinoFade(0xE1, 0x14, 0x40, 0x40, 0x40, 4);
        g_casino_timer = 0;
        g_casino_step = POKER_CHOOSE;
    }
}

void CasinoPokerCollect(void)
{
    if (!g_poker_palanim1->on) {
        CasinoStartPalAnim(g_poker_palanim1);
    }
    if (!g_poker_palanim2->on) {
        CasinoStartPalAnim(g_poker_palanim2);
    }
    if (g_casino_timer == 0) {
        CasinoTween(&D_80095B28, 8, 8, -0x10, -0x10, 8);
        CasinoTween(&D_80095BF4, 8, 8, -0x10, -0x10, 8);
        CasinoTween(&D_80095CC0, 8, 8, -0x10, -0x10, 8);
    }
    if (g_casino_timer == 8) {
        CasinoSpritesSetOn(0x277, 8, 0);
        CasinoSpritesSetOn(0x27F, 0xB, 0);
        CasinoSpritesSetOn(0x28A, 0xB, 0);
        CasinoTween(&D_80095A90, 0, 0, 0, 0x20, 0x10);
    }
    if (g_casino_timer == 0x10) {
        CasinoTween(&D_80095A90, 0, 0x20, 0, -0x20, 0x10);
        CasinoSplitDigits(g_casino_win);
    }
    if (g_casino_timer > 0x20) {
        if (g_casino_win && !(g_casino_frame & 1)) {
            CasinoPayStep(&g_casino_win);
        }
        if ((u_int)g_casino_money > 99999998) {
            g_casino_win = 0;
            g_casino_money = 99999999;
        }
    }
    if (g_casino_win == 0) {
        CasinoTween(&D_80095A90, 0, -0x20, 0, -0x10, 0x10);
        g_casino_timer = -1;
        g_casino_step = POKER_CLEAR;
    }
}

void CasinoSplitDigits(u_int n)
{
    g_casino_pay_digit = 0;
    g_casino_win_digits[0] = n % 100000000 / 10000000;
    g_casino_win_digits[1] = n % 10000000 / 1000000;
    g_casino_win_digits[2] = n % 1000000 / 100000;
    g_casino_win_digits[3] = n % 100000 / 10000;
    g_casino_win_digits[4] = n % 10000 / 1000;
    g_casino_win_digits[5] = n % 1000 / 100;
    g_casino_win_digits[6] = n % 100 / 10;
    g_casino_win_digits[7] = n % 10;
}

void CasinoPayStep(int *win)
{
    if (g_casino_win_digits[g_casino_pay_digit]) {
        g_casino_money += CasinoPow10(g_casino_win_digits[g_casino_pay_digit], 7 - g_casino_pay_digit);
        *win -= CasinoPow10(g_casino_win_digits[g_casino_pay_digit], 7 - g_casino_pay_digit);
        g_casino_win_digits[g_casino_pay_digit]--;
        SsSeqStop(g_casino_seqs[7]);
        SsSeqPlay(g_casino_seqs[7], 1, 1);
    } else {
        g_casino_pay_digit++;
    }
}

int CasinoPow10(u_char unused, u_char n)
{
    int p;

    switch (n) {
    case 7:
        p = 10000000;
        break;
    case 6:
        p = 1000000;
        break;
    case 5:
        p = 100000;
        break;
    case 4:
        p = 10000;
        break;
    case 3:
        p = 1000;
        break;
    case 2:
        p = 100;
        break;
    case 1:
        p = 10;
        break;
    case 0:
        p = 1;
        break;
    }
    return p;
}

void CasinoPokerLose(void)
{
    if (g_casino_timer == 0) {
        CasinoStartAnim(D_80095770, 0x20, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x10) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139F60, g_casino_main_vab);
    }
    if ((((g_casino_pad_trig & PAD_RIGHT) || (g_casino_pad_trig & PAD_LEFT) || (g_casino_pad_trig & PAD_UP) ||
          (g_casino_pad_trig & PAD_DOWN) || (g_casino_pad_trig & PAD_CIRCLE) || (g_casino_pad_trig & PAD_SQUARE) ||
          (g_casino_pad_trig & PAD_TRIANGLE) || (g_casino_pad_trig & PAD_CROSS) || (g_casino_pad_trig & PAD_L1) ||
          (g_casino_pad_trig & PAD_L2) || (g_casino_pad_trig & PAD_R1) || (g_casino_pad_trig & PAD_R2)) &&
         g_casino_timer > 0x10) ||
        g_casino_timer > 0x40) {
        g_casino_timer = -1;
        g_casino_step = POKER_CLEAR;
    }
}

void CasinoPokerClear(void)
{
    int   i;
    short row;

    if (g_casino_timer == 0) {
        CasinoSpritesSetOn(0x3C, 0x1E, 1);
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139E94, g_casino_main_vab);
        for (i = 0; i < POKER_CARDS; i++) {
            CasinoTween(&D_800957E4[i], -((10 - i) * 32), 0, 0, 0, 0x20);
            CasinoStartAnim(&g_casino_objs[i], 0x20, 0, 0, 0, -((10 - i) * 32), 0, 0, 0, 0, 0);
        }
    }
    if (g_casino_timer == 0x10 && !g_poker_rank) {
        CasinoStartAnim(D_80095770, 0x20, -0x800, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x2C) {
        CasinoTween(&D_800954C4, 0x10, -8, -0x20, 8, 8);
        CASINO_LAMPS.b0 = 1;
        CASINO_LAMPS.b4 = 0;
        CasinoShowLayout(&D_800957E4[0], 1);
        CasinoShowLayout(&D_800957E4[1], 1);
        CasinoShowLayout(&D_800957E4[2], 1);
        CasinoShowLayout(&D_800957E4[3], 1);
        CasinoShowLayout(&D_800957E4[4], 1);
        if (g_casino_bet < 5) {
            row = g_casino_bet - 1;
        } else {
            row = 4;
        }
        if (g_poker_rank) {
            CasinoQueueCluts(row * 10 + 0x11D, 10, 0x7CA4);
            CasinoQueueCluts(g_poker_hand_spr[g_poker_rank - 1], 1, 0x7CA4);
        }
        g_casino_timer = -1;
        g_casino_step = POKER_ENTER;
    }
}

void CasinoPokerDoubleUp(void)
{
    CasinoLayout *a[11] = { &D_80094940, &D_8009488C, &D_80094A08, &D_80094A48, &D_80094B48, &D_80094B60, &D_80094C20,
                            &D_80094C38, &D_80094C50, &D_80094C68, &D_80094C80 };
    CasinoLayout *b[6] = { &D_80094D9C, &D_80094E28[0], &D_80094E28[1], &D_80094E28[2], &D_80094E28[3], &D_80094E28[4] };
    CasinoLayout *c[14] = { &g_poker_jackpot_digits[0], &g_poker_jackpot_digits[1], &g_poker_jackpot_digits[2],
                            &g_poker_jackpot_digits[3], &g_poker_jackpot_digits[4], &g_poker_jackpot_digits[5],
                            &g_poker_jackpot_digits[6], &g_poker_jackpot_digits[7], &g_poker_jackpot_digits[8],
                            &D_80095164, &D_800951CC, &D_8009520C, &D_800952AC, &D_800952EC };
    CasinoLayout *d[13] = { &g_poker_money_digits[0], &g_poker_money_digits[1], &g_poker_money_digits[2],
                            &g_poker_money_digits[3], &g_poker_money_digits[4], &g_poker_money_digits[5],
                            &g_poker_money_digits[6], &g_poker_money_digits[7], &D_8009519C, &D_8009523C, &D_8009527C,
                            &D_8009531C, &D_8009535C };
    int i;

    if (g_casino_timer == 1) {
        CasinoTween(&D_80095A90, 8, 8, -0x10, -0x10, 8);
    }
    if (g_casino_timer == 8) {
        CasinoSpritesSetOn(0x26C, 0xB, 0);
    }
    CasinoPokerDoublePanels();
    if (g_casino_timer == 1) {
        for (i = 0; i < 14; i++) {
            CasinoTween(c[i], -0xA0, 0, 0, 0, 0x20);
        }
        for (i = 0; i < 13; i++) {
            CasinoTween(d[i], 0xA0, 0, 0, 0, 0x20);
        }
        CasinoStartAnim(D_800953E0, 0x20, 0, 0, 0, -0xA0, 0, 0, 0, 0, 0);
        CasinoStartAnim(D_80095470, 0x20, 0, 0, 0, 0xA0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x20) {
        CasinoSpritesSetOn(0x208, 9, 0);
        CasinoSpritesSetOn(0x211, 8, 0);
        CasinoSpritesSetOn(0x219, 4, 0);
        CasinoSpritesSetOn(0x21D, 4, 0);
        CasinoSpritesSetOn(0x221, 4, 0);
        CasinoSpritesSetOn(0x225, 4, 0);
        CasinoSpritesSetOn(0x229, 4, 0);
        CasinoSpritesSetOn(0x22B, 4, 0);
        CasinoSpritesSetOn(0x22D, 1, 0);
        CasinoSpritesSetOn(0x22E, 1, 0);
        CasinoSpritesSetOn(0x22F, 1, 0);
        CasinoSpritesSetOn(0x230, 1, 0);
        CasinoSpritesSetOn(0x231, 1, 0);
        CasinoSpritesSetOn(0x232, 1, 0);
    }
    if (g_casino_timer == 0x20) {
        CasinoSpritesSetOn(0x14, 0x1E, 1);
        for (i = 0; i < 6; i++) {
            CasinoTween(b[i], 0, 4, 0, -8, 4);
        }
    }
    if (g_casino_timer == 0x27) {
        CasinoPlaySeq(&g_casino_seqs[2], (u_long *)0x139E18, g_casino_main_vab);
        CasinoSpritesSetOn(0x14F, 5, 0);
        CasinoSpritesSetOn(0x154, 0x24, 0);
        CasinoSpritesSetOn(0x178, 0x24, 0);
        CasinoSpritesSetOn(0x19C, 0x24, 0);
        CasinoSpritesSetOn(0x1C0, 0x24, 0);
        CasinoSpritesSetOn(0x1E4, 0x24, 0);
        for (i = 0; i < 11; i++) {
            CasinoTween(a[i], 0, -0x80, 0, 0, 0x20);
        }
    }
    if (g_casino_timer == 0x3F) {
        CasinoSpritesSetOn(0xF9, 9, 0);
        CasinoSpritesSetOn(0xF5, 4, 0);
        CasinoSpritesSetOn(0x102, 0xA, 0);
        CasinoSpritesSetOn(0x10C, 4, 0);
        CasinoSpritesSetOn(0x119, 9, 0);
        CasinoSpritesSetOn(0x110, 4, 0);
        CasinoSpritesSetOn(0x11D, 0xA, 0);
        CasinoSpritesSetOn(0x127, 0xA, 0);
        CasinoSpritesSetOn(0x131, 0xA, 0);
        CasinoSpritesSetOn(0x13B, 0xA, 0);
        CasinoSpritesSetOn(0x145, 0xA, 0);
    }
    if (g_casino_timer == 0x3F) {
        for (i = 0; i < POKER_CARDS; i++) {
            if (g_poker_held[i] == 1) {
                CasinoTween(&D_800957E4[i], 0x10, 0, -0x20, -8, 8);
            }
        }
    }
    if (g_casino_timer == 0x47) {
        CasinoSpritesSetOn(0x8C, 5, 0);
        CasinoSpritesSetOn(0x3C, 0x1E, 1);
        for (i = 0; i < POKER_CARDS; i++) {
            CasinoStartAnim(&g_casino_objs[i], 0x20, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        }
    }
    if (g_casino_timer == 0x67) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139E94, g_casino_main_vab);
        for (i = 0; i < POKER_CARDS; i++) {
            CasinoStartAnim(&g_casino_objs[i], 0x20, 0, 0, 0, -((10 - i) * 32), 0, 0, 0, 0, 0);
        }
    }
    if (g_casino_timer > 0x87) {
        if (g_poker_double_game == 0) {
            CasinoTween(&D_80095B28, 0, -0x20, 0, -0x10, 0x10);
            CasinoBuildObj(&D_80095DE0);
            CasinoTexObj(&D_80095DE0);
            CasinoSetXform(D_80095DD0->xform);
            CasinoDrawObj(D_80095DD0);
        } else if (g_poker_double_game == 1) {
            CasinoTween(&D_80095BF4, 0, -0x20, 0, -0x10, 0x10);
        } else if (g_poker_double_game == 2) {
            CasinoTween(&D_80095CC0, 0, -0x20, 0, -0x10, 0x10);
        }
        CasinoShowLayout(&D_80095CF8, 0);
        CasinoShowLayout(&D_80095D30, 0);
        CasinoShowLayout(&D_80095D68, 0);
        CasinoShowLayout(&D_80095E14, 0);
        CasinoShowLayout(&D_80095E94[0], 0);
        CasinoShowLayout(&D_80095E94[1], 0);
        CasinoShowLayout(&D_80095E94[2], 0);
        CasinoShowLayout(&D_80095E94[3], 0);
        CasinoShowLayout(&D_80095E94[4], 0);
        CasinoShowLayout(&D_80095E94[5], 0);
        CasinoShowLayout(&D_80095E94[6], 0);
        CasinoShowLayout(&D_80095E94[7], 0);
        CasinoShowLayout(&D_80095FBC[0], 0);
        CasinoShowLayout(&D_80095FBC[1], 0);
        CasinoShowLayout(&D_80095FBC[2], 0);
        CasinoShowLayout(&D_80095FBC[3], 0);
        CasinoShowLayout(&D_80095FBC[4], 0);
        CasinoShowLayout(&D_80095FBC[5], 0);
        CasinoShowLayout(&D_80095FBC[6], 0);
        CasinoShowLayout(&D_80095FBC[7], 0);
        CasinoShowLayout(&D_800960E4[0], 0);
        CasinoShowLayout(&D_800960E4[1], 0);
        CasinoShowLayout(&D_800960E4[2], 0);
        CasinoShowLayout(&D_800960E4[3], 0);
        CasinoShowLayout(&D_800960E4[4], 0);
        CasinoShowLayout(&D_800960E4[5], 0);
        CasinoShowLayout(&D_800960E4[6], 0);
        CasinoShowLayout(&D_800960E4[7], 0);
        CasinoShowLayout(&D_800965E8, 0);
        CasinoShowLayout(&D_8009651C, 0);
        g_casino_timer = -1;
        g_casino_step = POKER_DOUBLE_PLAY;
    }
}
