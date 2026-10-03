/* Persona 1 (JP) - CASINO's video poker: ranking a hand.
 *   0x800723A4 CasinoPokerEvaluate
 *   0x8007259C CasinoPokerRankOf
 *   0x800726F8 CasinoPokerEvalInit
 *   0x80072728 CasinoPokerStraight
 *   0x80072894 CasinoPokerFlush
 *   0x8007293C CasinoPokerAddGroup
 *   0x800729DC CasinoSortBytes
 *   0x80072A7C CasinoPokerPick
 *   0x80072AF0 CasinoPokerCallRank
 *   0x80072C34 CasinoPokerWinMusic
 *
 * A card's rank is card % 13 (0 the ace) and its suit card / 13; suit 4 is
 * the joker, which stands in for whatever completes the best hand. Ranks:
 * 1 royal flush, 2 five of a kind, 3 straight flush, 4 four of a kind,
 * 5 full house, 6 flush, 7 straight, 8 three of a kind, 9 two pair.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>
#include <persona/casino/poker.h>

extern void CasinoPlaySeq(short *h, u_long *seq, short vab);
extern void func_80082658(); /* (u_char *out, int n, int range, int unique), called with three here */

/* What the hand holds: up to two groups of equal ranks and how many of
   each, and whether the five make a flush or a straight. */
typedef struct {
    u_char joker;
    u_char flush;
    u_char straight;
    u_char high;     /* where an ace-high straight starts */
    u_char groups;
    u_char rank[2];
    u_char count[2];
} PokerEval;

void   CasinoPokerEvalInit(PokerEval *e);
u_char CasinoPokerRankOf(PokerEval *e);
int    CasinoPokerStraight(PokerEval *e, u_char *rank);
void   CasinoPokerFlush(PokerEval *e, u_char *suit);
int    CasinoPokerAddGroup(PokerEval *e, u_char r);
void   CasinoSortBytes(int n, u_char *a);

u_char CasinoPokerEvaluate(u_char *cards, u_char *marks)
{
    u_char    rank[5];
    u_char    suit[5];
    PokerEval e;
    int       i;
    int       j;
    u_char    r;

    CasinoPokerEvalInit(&e);
    for (i = 0; i < POKER_CARDS; i++) {
        rank[i] = cards[i] % 13;
        suit[i] = cards[i] / 13;
        marks[i] = 0;
        if (suit[i] == 4) {
            e.joker = 1;
            rank[i] = 0xFF;
            marks[i] = 1;
        }
    }
    CasinoPokerFlush(&e, suit);
    if (e.flush != 1) {
        for (i = 0; i < 4; i++) {
            r = rank[i];
            for (j = i + 1; j < POKER_CARDS; j++) {
                if (r == rank[j]) {
                    CasinoPokerAddGroup(&e, r);
                    marks[i] = marks[j] = 1;
                    break;
                }
            }
        }
    } else {
        marks[0] = marks[1] = marks[2] = marks[3] = marks[4] = 1;
    }
    if (!e.groups) {
        CasinoPokerStraight(&e, rank);
    }
    if (e.straight == 1) {
        marks[0] = marks[1] = marks[2] = marks[3] = marks[4] = 1;
    }
    return CasinoPokerRankOf(&e);
}

u_char CasinoPokerRankOf(PokerEval *e)
{
    u_char r;

    r = 0;
    if (e->groups == 0) {
        if (e->straight == 1) {
            r = 7;
            if (e->flush == 1) {
                if (e->high == 9 || e->high == 10) {
                    r = 1;
                } else {
                    r = 3;
                }
            }
        } else if (e->flush == 1) {
            r = 6;
        }
    } else if (e->groups == 1) {
        switch (e->count[0]) {
        case 2:
            r = e->joker == 1 ? 8 : 0;
            break;
        case 3:
            r = e->joker == 1 ? 4 : 8;
            break;
        case 4:
            r = e->joker == 1 ? 2 : 4;
            break;
        }
    } else if (e->groups == 2) {
        switch (e->count[0]) {
        case 2:
            switch (e->count[1]) {
            case 2:
                r = e->joker == 1 ? 5 : 9;
                break;
            case 3:
                r = 5;
                break;
            }
            break;
        case 3:
            r = 5;
            break;
        }
    }
    return r;
}

void CasinoPokerEvalInit(PokerEval *e)
{
    e->rank[0] = 0xFF;
    e->rank[1] = 0xFF;
    e->joker = 0;
    e->flush = 0;
    e->straight = 0;
    e->high = 0;
    e->groups = 0;
    e->count[0] = 1;
    e->count[1] = 1;
}

int CasinoPokerStraight(PokerEval *e, u_char *rank)
{
    int    i;
    int    n;
    u_char gaps;
    u_char shift;
    int    next;

    n = 0;
    gaps = 0;
    shift = 0;
    CasinoSortBytes(5, rank);
    if (rank[0] == 0) {
        e->high = rank[1];
        if (rank[1] == 9 || rank[1] == 10) {
            shift = 8;
        } else {
            return 1;
        }
    } else if (rank[0] == 9 && e->joker) {
        e->high = rank[0];
    }
    for (i = 1; i < POKER_CARDS; i++) {
        if (rank[0] + i + gaps == rank[i] - shift) {
            n++;
        } else {
            if (!e->joker) {
                return 1;
            }
            next = gaps + 1;
            if (rank[0] + i + next == rank[i] - shift) {
                if (++gaps < 2) {
                    n++;
                } else {
                    return 1;
                }
            } else if (i == 4 && rank[4] == 0xFF) {
                n++;
            } else {
                return 1;
            }
        }
    }
    if (n == 4) {
        e->straight = 1;
    }
    return 1;
}

void CasinoPokerFlush(PokerEval *e, u_char *suit)
{
    int    i;
    int    n;
    u_char s;

    n = 0;
    if (suit[0] == 4) {
        s = suit[1];
        for (i = 2; i < POKER_CARDS; i++) {
            if (s == suit[i]) {
                n++;
            }
        }
    } else {
        s = suit[0];
        for (i = 1; i < POKER_CARDS; i++) {
            if (s == suit[i]) {
                n++;
            }
        }
    }
    if (n == 4) {
        e->flush = 1;
    }
    if (n == 3 && e->joker == 1) {
        e->flush = 1;
    }
}

int CasinoPokerAddGroup(PokerEval *e, u_char r)
{
    int i;

    if (e->groups) {
        for (i = 0; i < e->groups; i++) {
            if (e->rank[i] == r) {
                e->count[i]++;
                return 1;
            }
        }
    }
    e->rank[e->groups] = r;
    e->count[e->groups]++;
    e->groups++;
    return 1;
}

void CasinoSortBytes(int n, u_char *a)
{
    int    k;
    int    i;
    int    j;
    int    m;
    u_char t;
    u_char x;

    for (k = 0; k < n - 1; k++) {
        for (i = 0; i < n - 1; i++) {
            t = a[i];
            m = i;
            for (j = i + 1; j < n; j++) {
                if (a[j] < t) {
                    x = a[j];
                    a[j] = t;
                    a[m] = x;
                    m = j;
                }
            }
        }
    }
}

void CasinoPokerPick(int n, u_char *out, int count)
{
    int i;

    func_80082658(g_casino_deck, n, n);
    for (i = 0; i < count; i++) {
        out[i] = g_casino_deck[i];
    }
}

/* The dealer calls the hand. */
void CasinoPokerCallRank(u_char rank)
{
    switch (rank) {
    case 1:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139DA0, g_casino_main_vab);
        break;
    case 2:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139BC0, g_casino_vab);
        break;
    case 3:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139D64, g_casino_main_vab);
        break;
    case 4:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139B84, g_casino_vab);
        break;
    case 5:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139BFC, g_casino_vab);
        break;
    case 6:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139B48, g_casino_vab);
        break;
    case 7:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139CB0, g_casino_vab);
        break;
    case 8:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139CEC, g_casino_vab);
        break;
    case 9:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139D28, g_casino_vab);
        break;
    }
}

/* The win's music, by rank; returns the sequence it started. */
int CasinoPokerWinMusic(u_char rank)
{
    switch (rank) {
    case 1:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x139FB8, g_casino_main_vab);
        return 0x139FB8;
    case 2:
    case 3:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x13A208, g_casino_main_vab);
        return 0x13A208;
    case 4:
    case 5:
    case 6:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x13A338, g_casino_main_vab);
        return 0x13A338;
    case 7:
    case 8:
    case 9:
        CasinoPlaySeq(&g_casino_seqs[0], (u_long *)0x13A418, g_casino_main_vab);
        return 0x13A418;
    }
}
