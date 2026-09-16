/* Persona 1 (JP) - what a won fight is worth, shared out.  BTLP only.
 *   0x80096760 BtlBattleResults
 *
 * Run once as the fight is won, before the level-ups. Each member is weighed
 * by what its own spells took and by its share of the hp the party lost, and
 * the number is drawn on the board a won fight is shown on. The experience and
 * the second pot are then shared out against those weights - a third evenly
 * among the members still standing and the rest by weight - and each member's
 * two levels are checked against their curves: anyone whose experience covers
 * the next level raises the flag the level-ups walk on.
 *
 * A scripted fight skips the sharing: each member is simply taken to the next
 * level's own entry.
 *
 * The sharing is done in doubles, which is what makes the routine as long as
 * it is: every step of it is a soft-float call.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/char.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/number.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sides.h>

/* The highest either of a record's two levels goes. */
#define RESULTS_LEVEL_CAP 99

/* A third of the experience is shared evenly among the members still standing
   and the rest by weight. */
#define RESULTS_EVEN_PART 3.0

/* Six cells to a member's number on the board, five digits drawn into it. */
#define RESULTS_CELLS  6
#define RESULTS_DIGITS 5

/* Set on a fighter whose turn a script drove, which takes no share. */
#define RESULTS_SCRIPTED 0x10000000

/* The curve for Char.unk56, the same shape as the level curve. */
extern int   g_rank_exp[];
extern u_char g_btl_won_share_cells[];

/* Raised while any member's experience changed, which the board reads. */
extern u_char g_btl_exp_gained;

/* 67.41%. The two walks, the weights and both curves are the image's; what is
   left is where the doubles live - the image spills every one of them to a
   stack slot of its own and this keeps several in saved registers - and the
   first walk reaching the records by index rather than through a pointer the
   loop steps. */
#ifdef NON_MATCHING
void BtlBattleResults(void)
{
    double    hp;
    double    rounds;
    double    exp;
    double    members;
    double    total;
    double    pot;
    double    casts;
    double    mine;
    double    even;
    double    weighed;
    BtlActor *a;
    int       i;
    int       n;
    int       owed;
    int       rank;

    BtlAverageSides();
    if (g_btl_round == 0) {
        g_btl_round = 1;
    }
    if (g_btl_party_counted == 0) {
        g_btl_party_counted = 1;
    }
    hp = (double)(u_int)g_btl_won_hp;
    rounds = (double)(u_int)g_btl_round;
    members = (double)g_btl_party_counted;
    exp = (double)(u_int)g_btl_won_exp;
    pot = (double)(u_int)g_btl_won_unk10;
    total = 0.0;
    casts = (double)(u_int)g_btl_won_casts;

    for (i = 0; i < BTL_PARTY; i++) {
        if (g_btl_actors[i].c.key != 0) {
            n = (int)((double)(u_char)g_btl_actors[i].unkD0 * hp
                          / (rounds * members)
                      + (double)(u_int)g_btl_actors[i].damage_dealt);
            g_btl_actors[i].won_share = n;
            total += (double)n;
            BtlDrawNumberAlt(&g_btl_won_share_cells[i * RESULTS_CELLS], n,
                             RESULTS_DIGITS);
        }
    }

    g_btl_level_up = 0;
    g_btl_exp_gained = 0;
    for (i = 0; i < BTL_PARTY; i++) {
        a = &g_btl_actors[i];
        if (a->c.key == 0) {
            continue;
        }
        if (a->c.level < RESULTS_LEVEL_CAP) {
            if (g_btl_scripted != 0) {
                a->unk74 += g_level_exp[a->c.level] - a->c.unk14;
                a->c.unk10 += a->unk74;
                a->c.unk14 += a->unk74;
            } else {
                if ((signed char)a->c.status != BTL_STATUS_DOWN
                    && (a->flags & BTL_ACTOR_OUT) == 0) {
                    even = exp / (members * RESULTS_EVEN_PART);
                } else {
                    even = 0.0;
                }
                mine = (double)(u_int)a->won_share;
                if (total != 0.0) {
                    weighed = (exp + exp) / RESULTS_EVEN_PART * (mine / total);
                } else {
                    weighed = 0.0;
                }
                a->unk74 = (int)((double)(u_int)a->unk74 + (even + weighed));
                a->c.unk10 += a->unk74;
                a->c.unk14 += a->unk74;
                if (a->unk74 != 0) {
                    g_btl_exp_gained = 1;
                }
            }
            a->c.unk18 = g_level_exp[a->c.level] - a->c.unk14;
            if (a->c.unk18 <= 0) {
                a->level_up = 1;
                g_btl_level_up = 1;
            }
        }
        if (a->c.unk56 < RESULTS_LEVEL_CAP) {
            if (g_btl_scripted != 0) {
                owed = 0;
                for (rank = 0; rank < a->c.unk56; rank++) {
                    owed += g_rank_exp[rank + 1];
                }
                a->c.unk1C = owed;
                a->c.unk56++;
            } else if ((a->flags & RESULTS_SCRIPTED) == 0) {
                if (a->c.key != 0 && (signed char)a->c.status != BTL_STATUS_DOWN
                    && (a->flags & BTL_ACTOR_OUT) == 0) {
                    even = pot / (double)(g_btl_party_counted * 3);
                } else {
                    even = 0.0;
                }
                mine = (double)(u_int)a->won_share;
                if (total != 0.0) {
                    weighed = pot / RESULTS_EVEN_PART * (mine / total);
                } else {
                    weighed = 0.0;
                }
                if (casts != 0.0) {
                    n = (int)(even + weighed
                              + pot / RESULTS_EVEN_PART
                                    * ((double)(u_int)a->casts / casts));
                } else {
                    n = (int)(even + weighed);
                }
                a->unk78 = n;
                a->c.unk1C += n;
                owed = 0;
                for (rank = 0; rank < a->c.unk56; rank++) {
                    owed += g_rank_exp[rank + 1];
                }
                while (a->c.unk1C >= owed && a->c.unk56 < RESULTS_LEVEL_CAP) {
                    a->unk56_up = 1;
                    a->c.unk56++;
                    owed += g_rank_exp[a->c.unk56];
                }
            } else {
                a->unk78 = 0;
            }
        }
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/battleresults", BtlBattleResults);
#endif
