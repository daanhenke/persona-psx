/* Persona 1 (JP) - CASINO's video poker: the second double-up game,
 * high-low.
 *   0x800746C0 CasinoPokerHiLoRun
 *   0x80074808 CasinoPokerHiLoOpen
 *   0x800748F8 CasinoPokerHiLoDeal
 *   0x80074A1C CasinoPokerHiLoShowFirst
 *   0x80074C40 CasinoPokerHiLoShowNext
 *   0x80074DB4 CasinoPokerHiLoGuess
 *   0x80074E28 CasinoPokerHiLoReveal
 *   0x80074F04 CasinoPokerHiLoWin
 *   0x8007510C CasinoPokerHiLoAgain
 *   0x800752AC CasinoPokerHiLoLeave
 *   0x80075310 CasinoPokerHiLoCollect
 *   0x8007547C CasinoPokerHiLoLose
 *   0x800756C8 CasinoPokerHiLoEnd
 *   0x80075848 CasinoPokerHiLoNext
 *   0x800759C0 CasinoPokerHiLoDealFirst
 *   0x80075C48 CasinoPokerHiLoDealNext
 *   0x80075EF4 CasinoPokerHiLoOdds
 *
 * One card is up; the player calls whether the next is higher or lower.
 * The cards alternate between the two slots round by round. The bigger
 * the stake, the likelier the deal is quietly redrawn to an equal rank,
 * which loses - though the odds are a signed byte, so from 0x80 up they
 * are never met.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libsnd.h>
#include <persona/casino/casino.h>
#include <persona/casino/poker.h>

extern void CasinoTween();        /* (CasinoLayout *l, short dx, short dy, short dw, short dh, short frames) */
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

extern int    rand(void);
extern void   func_80076894(void);
extern void   func_8007756C(void);
extern void   func_80077164(void);
extern void   func_80077288(void);
extern void   func_80077F38(void);
extern void   func_80078014(void);
extern void   func_80078164(void);
extern void   func_80082658(u_char *out, int n, int range, int unique);
extern short  func_80082454(u_char card);
extern void   func_800824CC(int i, u_char card, int face, int spr);
extern void   func_800825A4(u_char card, int spr);
extern int    func_800827EC(); /* (u_char a, u_char b), called unprototyped here */

extern CasinoXform  g_casino_xforms[25];
extern CasinoLayout D_80096660; /* the two calls */
extern CasinoLayout D_800966F8;

extern u_char g_poker_hilo_round; /* which slot holds the card up */
extern u_char g_poker_hilo_guess; /* the call made, 0xFF none     */

void CasinoPokerHiLoOpen(void);
void CasinoPokerHiLoDeal(void);
void CasinoPokerHiLoShowFirst(void);
void CasinoPokerHiLoShowNext(void);
void CasinoPokerHiLoGuess(void);
void CasinoPokerHiLoReveal(void);
void CasinoPokerHiLoWin(void);
void CasinoPokerHiLoAgain(void);
void CasinoPokerHiLoLeave(void);
void CasinoPokerHiLoCollect(void);
void CasinoPokerHiLoLose(void);
void CasinoPokerHiLoEnd(void);
void CasinoPokerHiLoNext(void);
void CasinoPokerHiLoDealFirst(void);
void CasinoPokerHiLoDealNext(void);
s8   CasinoPokerHiLoOdds(int stake);

void CasinoPokerHiLoRun(void)
{
    switch (g_poker_double_step) {
    case 0x11:
        CasinoPokerHiLoOpen();
        break;
    case 0x12:
        CasinoPokerHiLoDeal();
        break;
    case 0x34:
        if (!g_poker_hilo_round) {
            CasinoPokerHiLoShowFirst();
        } else {
            CasinoPokerHiLoShowNext();
        }
        break;
    case 0x35:
        CasinoPokerHiLoGuess();
        break;
    case 0x36:
        CasinoPokerHiLoReveal();
        break;
    case 0x16:
        CasinoPokerHiLoWin();
        break;
    case 0x37:
        CasinoPokerHiLoAgain();
        break;
    case 0x38:
        CasinoPokerHiLoLeave();
        break;
    case 0x31:
        CasinoPokerHiLoCollect();
        break;
    case 0x17:
        CasinoPokerHiLoLose();
        break;
    case 0x3A:
        if (g_poker_rank == 0x38) {
            CasinoPokerHiLoNext();
        } else {
            CasinoPokerHiLoEnd();
        }
        break;
    case 0x13:
        func_80076894();
        break;
    }
}

void CasinoPokerHiLoOpen(void)
{
    if (g_casino_timer == 0) {
        g_poker_double_shown = 0;
        g_poker_double_pay = g_poker_payout * 2;
        if (g_poker_double_pay > 999999) {
            g_poker_double_pay = 1000000;
        }
        g_poker_hilo_round = 0;
        func_8007756C();
        CasinoTween(&D_8009651C, 0, 0x10, 0, 0, 1);
        CasinoTween(&D_800965E8, 0, 8, 0, 0, 1);
        CasinoShowLayout(&D_80096660, 0);
        CasinoShowLayout(&D_800966F8, 0);
        g_casino_timer = -1;
        g_poker_double_step = 0x12;
    }
}

void CasinoPokerHiLoDeal(void)
{
    if (!g_poker_hilo_round) {
        CasinoPokerHiLoDealFirst();
    } else {
        CasinoPokerHiLoDealNext();
    }
    D_800962E4->xform->rot.vy = -0x800;
    D_800962E4->xform->trans.vx = 0x31;
    D_800962E4->xform->trans.vy = 0x50;
    D_80096240->xform->rot.vy = -0x800;
    D_80096240->xform->trans.vx = 0x31;
    D_80096240->xform->trans.vy = 0x50;
    D_80095770->xform->rot.vy = 0;
    D_80095770->xform->rot.vx = 0x800;
    D_80095770->xform->trans.vx = 0;
    D_80095770->xform->trans.vy = 0x60;
    g_poker_hilo_guess = 0xFF;
    D_80096484->xform->rot.vy = -0x800;
    D_80096484->xform->trans.vx = 0x31;
    D_80096484->xform->trans.vy = 0x50;
    g_poker_double_step = 0x34;
    g_casino_timer = -1;
}

void CasinoPokerHiLoShowFirst(void)
{
    if (g_casino_timer == 0) {
        CasinoStartAnim(&g_casino_objs[0], 0x20, 0, 0, 0, -0x140, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x20) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[1], 0x20, 0, 0, 0, -0x140, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x40) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139F38, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[0], 0x20, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        CasinoTween(&D_80096660, 0x140, 0, 0, 0, 0x20);
        CasinoTween(&D_800966F8, -0x140, 0, 0, 0, 0x20);
    }
    if (g_casino_timer == 0x44) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139E40, g_casino_main_vab);
    }
    if (g_casino_timer == 0x60 && !g_poker_double_shown) {
        func_80077164();
    }
    if (g_casino_timer > 0x68) {
        if (!g_poker_double_shown) {
            func_80077288();
        }
        g_poker_double_shown = 1;
        g_poker_double_step = 0x35;
        g_casino_timer = -1;
    }
}

void CasinoPokerHiLoShowNext(void)
{
    if (g_casino_timer == 0) {
        CasinoStartAnim(&g_casino_objs[(g_poker_hilo_round + 1) % 2], 0x20, 0, 0, 0, -0x140, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x20) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139F38, g_casino_main_vab);
        CasinoTween(&D_80096660, 0x140, 0, 0, 0, 0x20);
        CasinoTween(&D_800966F8, -0x140, 0, 0, 0, 0x20);
    }
    if (g_casino_timer == 0x40 && !g_poker_double_shown) {
        func_80077164();
    }
    if (g_casino_timer > 0x48) {
        if (!g_poker_double_shown) {
            func_80077288();
        }
        g_poker_double_shown = 1;
        g_poker_double_step = 0x35;
        g_casino_timer = -1;
    }
}

void CasinoPokerHiLoGuess(void)
{
    func_80078164();
    if (g_poker_hilo_guess != 0xFF) {
        SsSeqStop(g_casino_seqs[4]);
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        g_poker_double_step = 0x36;
        g_casino_timer = -1;
    }
}

void CasinoPokerHiLoReveal(void)
{
    u_char guess;

    CasinoStartAnim(&g_casino_objs[(g_poker_hilo_round + 1) % 2], 0x20, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
    guess = g_poker_hilo_guess;
    if (guess == func_800827EC(g_poker_hand[g_poker_hilo_round % 2], g_poker_hand[(g_poker_hilo_round + 1) % 2])) {
        g_poker_double_step = g_poker_rank = 0x16;
    } else {
        g_poker_double_step = g_poker_rank = 0x17;
    }
    g_casino_timer = -1;
}

void CasinoPokerHiLoWin(void)
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
        if (g_poker_hilo_guess == 0x16) {
            CasinoTween(&D_80096660, -0x140, 0, 0, 0, 0x20);
        } else if (g_poker_hilo_guess == 0x17) {
            CasinoTween(&D_800966F8, 0x140, 0, 0, 0, 0x20);
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

void CasinoPokerHiLoAgain(void)
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

void CasinoPokerHiLoLeave(void)
{
    if (g_casino_timer > 0x20) {
        CasinoTween(&D_8009651C, -0x140, 0, 0, 0, 0x20);
        g_casino_timer = -1;
        g_poker_double_step = 0x3A;
    }
}

void CasinoPokerHiLoCollect(void)
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
        if (g_poker_hilo_guess == 0x16) {
            CasinoTween(&D_80096660, -0x140, 0, 0, 0, 0x20);
        } else if (g_poker_hilo_guess == 0x17) {
            CasinoTween(&D_800966F8, 0x140, 0, 0, 0, 0x20);
        }
        CasinoTween(&D_800965E8, 0x140, 0, 0, 0, 0x20);
        g_casino_timer = -1;
        g_poker_double_step = 0x3A;
    }
    CasinoShowNumber(g_casino_money, D_80095FBC, 8);
    CasinoShowNumber(g_poker_payout, D_80095E94, 8);
    CasinoShowNumber(0, D_800960E4, 8);
}

void CasinoPokerHiLoLose(void)
{
    if (g_casino_timer == 0x10) {
        CasinoStartAnim(D_80096240, 0x10, 0, -0x800, 0, 0, -0x40, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x20) {
        CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x139C74, g_casino_vab);
        if (g_poker_hilo_guess == 0x16) {
            CasinoTween(&D_80096660, 0, 0, 0, -0x10, 0x10);
        } else if (g_poker_hilo_guess == 0x17) {
            CasinoTween(&D_800966F8, 0, 0, 0, -0x10, 0x10);
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

void CasinoPokerHiLoEnd(void)
{
    if (g_casino_timer == 0) {
        CasinoStartAnim(&g_casino_objs[0], 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
        CasinoStartAnim(&g_casino_objs[1], 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x10) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139E94, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[0], 0x20, 0, 0, 0, -0x100, 0, 0, 0, 0, 0);
        CasinoStartAnim(&g_casino_objs[1], 0x20, 0, 0, 0, -0x100, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer > 0x30) {
        if (g_poker_rank == 0x17 || g_poker_rank == 0x31) {
            g_poker_double_game = 0x39;
        }
        g_casino_timer = -1;
    }
}

void CasinoPokerHiLoNext(void)
{
    if (g_casino_timer == 0) {
        CasinoStartAnim(&g_casino_objs[g_poker_hilo_round % 2], 0x10, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 0x10) {
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139E94, g_casino_main_vab);
        CasinoStartAnim(&g_casino_objs[g_poker_hilo_round % 2], 0x20, 0, 0, 0, -0x100, 0, 0, 0, 0, 0);
        CasinoStartAnim(&g_casino_objs[(g_poker_hilo_round + 1) % 2], 0x20, 0, 0, 0, -0x60, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer > 0x30) {
        g_poker_double_step = 0x12;
        g_casino_timer = -1;
        g_poker_hilo_round++;
    }
}

void CasinoPokerHiLoDealFirst(void)
{
    s8    odds;
    int   i;
    short face;

    do {
        func_80082658(g_poker_deck, 2, 0x34, 1);
    } while (g_poker_deck[0] == 0x34 || g_poker_deck[1] == 0x34);
    odds = CasinoPokerHiLoOdds(g_poker_double_pay);
    if (odds && rand() % 256 < odds) {
        for (;;) {
            u_char r0;
            u_char r1;

            if (g_poker_deck[0] != g_poker_deck[1]) {
                r0 = g_poker_deck[0] % 13;
                r1 = g_poker_deck[1] % 13;
                if (r0 == r1) {
                    break;
                }
            }
            func_80082658(g_poker_deck, 2, 0x34, 1);
        }
    }
    for (i = 0; i < 2; i++) {
        g_poker_hand[i] = g_poker_deck[i];
        face = func_80082454(g_poker_hand[i]);
        func_800824CC(i, g_poker_hand[i], face, i * 0x15 + 0x8D);
        func_800825A4(g_poker_hand[i], i * 0x15 + 0x8D);
        g_casino_xforms[i].rot.vx = 0;
        g_casino_xforms[i].rot.vy = 0x800;
        g_casino_xforms[i].rot.vz = 0;
        g_casino_xforms[i].trans.vy = 0x10;
        g_casino_xforms[i].trans.vz = 0x200;
        g_casino_xforms[i].scale.vx = 0x1000;
        g_casino_xforms[i].scale.vy = 0x1000;
        g_casino_xforms[i].scale.vz = 0x1000;
        if (i == 0) {
            g_casino_xforms[i].trans.vx = 0x110;
        } else {
            g_casino_xforms[i].trans.vx = 0x170;
        }
        CasinoSetXform(&g_casino_xforms[i]);
        CasinoDrawObj(&g_casino_objs[i]);
    }
}

void CasinoPokerHiLoDealNext(void)
{
    int     n;
    u_char *deck;
    s8    odds;
    short face;

    deck = g_poker_deck;
    n = (g_poker_hilo_round + 1) % 2;
    do {
        do {
            func_80082658(deck, 1, 0x34, 1);
        } while (deck[0] == 0x34);
    } while ((g_poker_hand[n] = deck[0]) == g_poker_hand[!n]);
    odds = CasinoPokerHiLoOdds(g_poker_double_pay);
    if (odds && rand() % 256 < odds) {
        for (;;) {
            u_char r0;
            u_char r1;

            if (g_poker_hand[n] != g_poker_hand[!n]) {
                r0 = g_poker_hand[n] % 13;
                r1 = g_poker_hand[!n] % 13;
                if (r0 == r1) {
                    break;
                }
            }
            func_80082658(deck, 1, 0x34, 1);
            g_poker_hand[n] = deck[0];
        }
    }
    face = func_80082454(g_poker_hand[n]);
    func_800824CC(n, g_poker_hand[n], face, n * 0x15 + 0x8D);
    func_800825A4(g_poker_hand[n], n * 0x15 + 0x8D);
    g_casino_xforms[n].rot.vy = 0x800;
    g_casino_xforms[n].trans.vy = 0x10;
    g_casino_xforms[n].trans.vz = 0x200;
    g_casino_xforms[n].scale.vx = 0x1000;
    g_casino_xforms[n].scale.vy = 0x1000;
    g_casino_xforms[n].scale.vz = 0x1000;
    g_casino_xforms[n].rot.vx = 0;
    g_casino_xforms[n].rot.vz = 0;
    g_casino_xforms[n].trans.vx = 0x170;
    CasinoSetXform(&g_casino_xforms[n]);
    CasinoDrawObj(&g_casino_objs[n]);
}

/* How often, out of 256, the deal is redrawn to a tie. */
s8 CasinoPokerHiLoOdds(int stake)
{
    s8 odds;

    odds = 0;
    if (stake >= 0 && stake <= 100) {
        odds = 0;
    } else if (stake > 100 && stake <= 300) {
        odds = 2;
    } else if (stake > 300 && stake <= 500) {
        odds = 4;
    } else if (stake > 500 && stake <= 700) {
        odds = 8;
    } else if (stake > 700 && stake <= 1000) {
        odds = 0x10;
    } else if (stake > 1000 && stake <= 1500) {
        odds = 0x20;
    } else if (stake > 1500 && stake <= 2000) {
        odds = 0x30;
    } else if (stake > 2000 && stake <= 3000) {
        odds = 0x40;
    } else if (stake > 3000 && stake <= 5000) {
        odds = 0x50;
    } else if (stake > 5000 && stake <= 8000) {
        odds = 0x60;
    } else if (stake > 8000 && stake <= 12000) {
        odds = 0x70;
    } else if (stake > 12000 && stake <= 18000) {
        odds = 0x80;
    } else if (stake > 18000 && stake <= 26000) {
        odds = 0x80;
    } else if (stake > 26000 && stake <= 36000) {
        odds = 0x80;
    } else if (stake > 36000 && stake <= 50000) {
        odds = 0x80;
    } else if (stake > 50000 && stake <= 70000) {
        odds = 0x80;
    } else if (stake > 70000 && stake <= 100000) {
        odds = 0x80;
    } else if (stake > 100000 && stake <= 150000) {
        odds = 0x80;
    } else if (stake > 150000 && stake <= 220000) {
        odds = 0x80;
    } else if (stake > 220000 && stake <= 320000) {
        odds = 0x80;
    } else if (stake > 320000) {
        odds = 0x80;
    }
    return odds;
}
