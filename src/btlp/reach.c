/* Persona 1 (JP) - the reach grid: which fighters a move can land on.
 * BTLP only.
 *   0x80094550 BtlMarkMoveArea       0x800946E8 BtlMarkEnemiesAround
 *   0x8009485C BtlPickAiTarget       0x80094A20 BtlMarkPartyAround
 *   0x80094BE8 BtlPickableMask       0x80094C40 BtlPartyPickableMask
 *
 * g_btl_reach is the field seen from above, five lanes across and fifteen
 * rows deep. The enemies stand in rows 0..4 and the party in rows 10..14 - an
 * object keeps its row within its own side, so a member's is put ten further
 * on - and rows 5..9 are the empty ground between the two.
 *
 * The four markers each clear the grid, light the cells a shape covers, and
 * then mark every fighter standing on a lit cell as pickable, answering with
 * the deepest row they marked or -1 when they marked nobody. The two masks
 * turn those marks into the target bits a turn is played with.
 *
 * A shape is a run of five-bit rows, REACH_BIT0 being the leftmost lane, and
 * there are two kinds of them:
 *
 * - A line, nine rows out of g_btl_reach_lines, is thrown from the row just in
 *   front of the thrower toward the other side. The empty ground is stepped
 *   over without using up a row of the shape. BtlMarkMoveArea throws one from
 *   a party member at the enemies and BtlPickAiTarget from an enemy at the
 *   party; both centre it on the thrower's lane, unless a member's swing asks
 *   for it to start at the leftmost one.
 * - A burst, five rows out of g_btl_reach_bursts, is centred on a fighter.
 *   BtlMarkEnemiesAround centres one on an enemy and marks enemies;
 *   BtlMarkPartyAround centres one on whichever fighter it is handed and marks
 *   the party.
 *
 * The party's two routines pass over anyone down or out of the fight, and the
 * line thrown at the party over anyone lifted off the floor or held by a
 * puppet string as well; the burst passes over the puppet but not the lifted.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* The bit for a shape row's leftmost lane, and what a lit cell holds. */
#define REACH_BIT0 0x10
#define REACH_LIT  0xFF

/* A member's swing with this set throws its line from the leftmost lane
   rather than centring it. */
#define REACH_FROM_EDGE 0x80

#define REACH_LANE(o) ((o)->col2 >> 1)

int BtlMarkMoveArea(BtlActor *a, int shape, int flags)
{
    u_char       *p;
    BtlActor     *e;
    BtlObj       *o;
    int           first;
    int           row;
    int           lane;
    u_int         bit;
    int           hit;
    int           deepest;
    int           i;

    p = (u_char *)g_btl_reach;
    i = REACH_CELLS - 1;
    do {
        *p = 0;
        i--;
        p++;
    } while (i >= 0);

    p = g_btl_reach_lines[shape];
    first = 0;
    if ((flags & REACH_FROM_EDGE) == 0) {
        first = REACH_LANE(a->obj) - 2;
    }
    for (row = a->obj->row + REACH_PARTY_ROW - 1; row >= 0; row--, p++) {
        for (lane = first, bit = REACH_BIT0; lane < REACH_LANES; lane++) {
            if (row >= REACH_GAP_ROW && row < REACH_PARTY_ROW) {
                p--;
                break;
            }
            hit = bit & *p;
            bit >>= 1;
            if (hit != 0 && lane >= 0) {
                g_btl_reach[lane][row] = REACH_LIT;
            }
        }
    }

    i = 0;
    deepest = -1;
    do {
        g_btl_combatants[i].pickable = 0;
        e = &g_btl_combatants[i];
        o = e->obj;
        if (e->c.key != 0
            && g_btl_reach[REACH_LANE(o)][o->row] != 0) {
            e->pickable = 1;
            if (deepest < o->row) {
                deepest = o->row;
            }
        }
        i++;
    } while (i < BTL_ENEMIES);
    return deepest;
}

int BtlMarkEnemiesAround(BtlActor *at, int shape)
{
    u_char       *p;
    BtlActor     *e;
    BtlObj       *o;
    int           left;
    int           first;
    int           row;
    int           lane;
    u_int         bit;
    int           hit;
    int           deepest;
    int           i;

    p = (u_char *)g_btl_reach;
    i = REACH_CELLS - 1;
    do {
        *p = 0;
        i--;
        p++;
    } while (i >= 0);

    o = at->obj;
    p = g_btl_reach_bursts[shape];
    left = REACH_LANE(o) - 2;
    first = o->row - 2;
    for (row = first; row <= first + 4; row++, p++) {
        if (row >= REACH_ROWS) {
            break;
        }
        for (lane = left, bit = REACH_BIT0; lane < REACH_LANES; lane++) {
            hit = bit & *p;
            bit >>= 1;
            if (hit != 0 && lane >= 0 && row >= 0) {
                g_btl_reach[lane][row] = REACH_LIT;
            }
        }
    }

    i = 0;
    deepest = -1;
    do {
        g_btl_combatants[i].pickable = 0;
        e = &g_btl_combatants[i];
        first = e->obj->row;
        left = REACH_LANE(e->obj);
        if (e->c.key != 0 && g_btl_reach[left][first] != 0) {
            e->pickable = 1;
            if (deepest < first) {
                deepest = first;
            }
        }
        i++;
    } while (i < BTL_ENEMIES);
    return deepest;
}

/* 99.38%, one instruction out, and the same one pickable.c's
   BtlAnyMemberTargetable is out by: the image loads the ailment into a scratch,
   compares that, and fills the branch's delay slot with the move into the
   register it keeps it in for the range test; gcc here loads straight into
   that register and leaves the slot empty. An ailment local assigned inside
   the test, the same local as the first loop's row, and the byte read twice
   with no local at all come out the same. */
#ifdef NON_MATCHING
int BtlPickAiTarget(BtlActor *a, int shape)
{
    /* Eight bytes of locals the routine reserves and never writes. */
    long          unused[2];
    u_char       *p;
    BtlObj       *o;
    int           status;
    int           left;
    int           first;
    int           row;
    int           lane;
    u_int         bit;
    int           hit;
    int           deepest;
    int           i;

    p = (u_char *)g_btl_reach;
    i = REACH_CELLS - 1;
    do {
        *p = 0;
        i--;
        p++;
    } while (i >= 0);

    o = a->obj;
    p = g_btl_reach_lines[shape];
    left = REACH_LANE(o) - 2;
    first = o->row + 1;
    for (row = first; row < REACH_ROWS; row++, p++) {
        for (lane = left, bit = REACH_BIT0; lane < REACH_LANES; lane++) {
            if (row >= REACH_GAP_ROW && row < REACH_PARTY_ROW) {
                p--;
                break;
            }
            hit = bit & *p;
            bit >>= 1;
            if (hit != 0 && lane >= 0) {
                g_btl_reach[lane][row] = REACH_LIT;
            }
        }
    }

    i = 0;
    deepest = -1;
    do {
        g_btl_actors[i].pickable = 0;
        left = REACH_LANE(g_btl_actors[i].obj);
        first = g_btl_actors[i].obj->row + REACH_PARTY_ROW;
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && g_btl_reach[left][first] != 0
            && (u_int)((signed char)g_btl_actors[i].c.status
                       - BTL_STATUS_LIFTED) >= 2) {
            g_btl_actors[i].pickable = 1;
            if (deepest < first) {
                deepest = first;
            }
        }
        i++;
    } while (i < BTL_PARTY);
    return deepest;
}
#else
INCLUDE_ASM("btlp/nonmatchings/reach", BtlPickAiTarget);
#endif

int BtlMarkPartyAround(BtlActor *at, int shape)
{
    u_char       *p;
    BtlObj       *o;
    int           status;
    int           left;
    int           first;
    int           row;
    int           lane;
    u_int         bit;
    int           hit;
    int           deepest;
    int           i;

    p = (u_char *)g_btl_reach;
    i = REACH_CELLS - 1;
    do {
        *p = 0;
        i--;
        p++;
    } while (i >= 0);

    o = at->obj;
    p = g_btl_reach_bursts[shape];
    left = REACH_LANE(o) - 2;
    first = o->row - 2;
    if (o->mark_num < BTL_PARTY) {
        first = o->row + REACH_PARTY_ROW - 2;
    }
    for (row = first; row < first + 5; row++, p++) {
        if (row >= REACH_ROWS) {
            break;
        }
        for (lane = left, bit = REACH_BIT0; lane < REACH_LANES; lane++) {
            hit = bit & *p;
            bit >>= 1;
            if (hit != 0 && lane >= 0 && row >= 0) {
                g_btl_reach[lane][row] = REACH_LIT;
            }
        }
    }

    i = 0;
    deepest = -1;
    do {
        g_btl_actors[i].pickable = 0;
        left = REACH_LANE(g_btl_actors[i].obj);
        first = g_btl_actors[i].obj->row + REACH_PARTY_ROW;
        if (g_btl_actors[i].c.key != 0
            && (status = (signed char)g_btl_actors[i].c.status)
                   != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && status != BTL_STATUS_NOINPUT
            && g_btl_reach[left][first] != 0) {
            g_btl_actors[i].pickable = 1;
            if (deepest < first) {
                deepest = first;
            }
        }
        i++;
    } while (i < BTL_PARTY);
    return deepest;
}

u_long BtlPickableMask(void)
{
    u_long mask;
    int    i;

    i = 0;
    mask = 0;
    do {
        if (g_btl_combatants[i].c.key != 0
            && g_btl_combatants[i].pickable != 0) {
            mask |= 1 << (i + BTL_PARTY);
        }
        i++;
    } while (i < BTL_ENEMIES);
    return mask;
}

u_long BtlPartyPickableMask(void)
{
    u_long mask;
    int    i;

    i = 0;
    mask = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[i].pickable != 0) {
            mask |= 1 << i;
        }
        i++;
    } while (i < BTL_PARTY);
    return mask;
}
