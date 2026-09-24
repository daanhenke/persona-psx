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

/* The first walk reaches the records through a pointer and the second by
   index, which is how the image addresses them. Every conversion but the
   globals' is signed. */
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
    double    cast;
    double    spent;
    BtlActor *a;
    int       i;
    int       n;
    int       owed;
    int       rank;
    int      *next;
    int       top;

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
        a = &g_btl_actors[i];
        if (a->c.key != 0) {
            double dealt = (double)a->damage_dealt;
            n = (int)((double)a->unkD0 * hp / (rounds * members) + dealt);
            a->won_share = n;
            total += (double)n;
            BtlDrawNumberAlt(&g_btl_won_share_cells[i * RESULTS_CELLS], n,
                             RESULTS_DIGITS);
        }
    }

    g_btl_level_up = 0;
    g_btl_exp_gained = 0;
    for (i = 0; i < BTL_PARTY; i++) {
        if (g_btl_actors[i].c.key == 0) {
            continue;
        }
        if (g_btl_actors[i].c.level < RESULTS_LEVEL_CAP) {
            next = &g_level_exp[g_btl_actors[i].c.level];
            if (g_btl_scripted != 0) {
                g_btl_actors[i].unk74 += *next - g_btl_actors[i].c.unk14;
                g_btl_actors[i].c.unk10 += g_btl_actors[i].unk74;
                g_btl_actors[i].c.unk14 += g_btl_actors[i].unk74;
            } else {
                if ((signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                    even = exp / (members * RESULTS_EVEN_PART);
                } else {
                    even = 0.0;
                }
                mine = (double)g_btl_actors[i].won_share;
                if (total != 0.0) {
                    weighed = (exp + exp) / RESULTS_EVEN_PART * (mine / total);
                } else {
                    weighed = 0.0;
                }
                g_btl_actors[i].unk74 = (int)((double)g_btl_actors[i].unk74
                                              + (even + weighed));
                g_btl_actors[i].c.unk10 += g_btl_actors[i].unk74;
                g_btl_actors[i].c.unk14 += g_btl_actors[i].unk74;
                if (g_btl_actors[i].unk74 != 0) {
                    g_btl_exp_gained = 1;
                }
            }
            g_btl_actors[i].c.unk18 = *next - g_btl_actors[i].c.unk14;
            if (g_btl_actors[i].c.unk18 <= 0) {
                g_btl_actors[i].level_up = 1;
                g_btl_level_up = 1;
            }
        }
        if (g_btl_actors[i].c.unk56 < RESULTS_LEVEL_CAP) {
            if (g_btl_scripted != 0) {
                owed = 0;
                for (rank = 0; rank < g_btl_actors[i].c.unk56; rank++) {
                    owed += g_rank_exp[rank + 1];
                }
                g_btl_actors[i].c.unk1C = owed;
                g_btl_actors[i].c.unk56++;
            } else if ((g_btl_actors[i].flags & RESULTS_SCRIPTED) == 0) {
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                    even = pot / (double)(g_btl_party_counted * 3);
                } else {
                    even = 0.0;
                }
                mine = (double)g_btl_actors[i].won_share;
                cast = (double)g_btl_actors[i].casts;
                if (total != 0.0) {
                    weighed = pot / RESULTS_EVEN_PART * (mine / total);
                } else {
                    weighed = 0.0;
                }
                if (casts != 0.0) {
                    spent = pot / RESULTS_EVEN_PART * (cast / casts);
                } else {
                    spent = 0.0;
                }
                g_btl_actors[i].unk78 = (int)(even + weighed + spent);
                g_btl_actors[i].c.unk1C += g_btl_actors[i].unk78;
                owed = 0;
                top = g_btl_actors[i].c.unk56;
                for (rank = 0; rank < top; rank++) {
                    owed += g_rank_exp[rank + 1];
                }
                while (g_btl_actors[i].c.unk1C >= owed
                       && g_btl_actors[i].c.unk56 < RESULTS_LEVEL_CAP) {
                    g_btl_actors[i].unk56_up = 1;
                    g_btl_actors[i].c.unk56++;
                    owed += g_rank_exp[g_btl_actors[i].c.unk56];
                }
            } else {
                g_btl_actors[i].unk78 = 0;
            }
        }
    }
}
