/* Persona 1 (JP) - a Persona growing, and a fighter losing a level.
 * BTLP only.
 *   0x80097158 BtlPersonaGrow  0x80097484 BtlDrainLevel
 *
 * BtlPersonaGrow runs as a member's Persona is put away after being cast. It
 * gains experience - three points, or less the further its level is above the
 * enemies' - and once it has enough for its rank it ranks up: another slot of
 * its spell order opens, the experience over is carried into the next rank,
 * and one column of its growth row is added to each of its five stats and to
 * the two numbers after them. Stats are held between 1 and 99 and the two
 * numbers between 1 and 999. A Persona already at its last rank, or one
 * marked as not growing, is left alone.
 *
 * BtlDrainLevel takes a level off a fighter. The protagonist loses three from
 * the highest of the base stats and four from both maxima; anyone else loses
 * the hit points and the stats their own growth tables gave them for that
 * level, and four from the spell points. Hit and spell points are held under
 * the new maxima, and the experience is worked out again from the table for
 * the level the fighter is left at - at level 1 that is all that happens.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/stats.h>

/* Ranks a Persona has, and what each stat and number is held to. */
#define GROW_RANKS    8
#define GROW_STAT_MAX 99
#define GROW_NUM_MAX  999

/* A growth row: seven columns - the five stats and the two numbers - of seven
   entries, one per rank from the second. */
#define GROW_COLUMNS 7
#define GROW_FIRST   2

/* The protagonist, and what a drained level takes from anyone. */
#define DRAIN_HERO     1
#define DRAIN_STAT     3
#define DRAIN_MAXIMA   4
#define DRAIN_FIRST    2
#define DRAIN_HP_ROW   50
#define DRAIN_STAT_ROW 250

extern int     g_btl_enemy_level;
extern u_short g_btl_persona_rank_exp[GROW_RANKS];
extern u_char  g_btl_persona_growth[][GROW_COLUMNS * GROW_COLUMNS];

void BtlPersonaGrow(BtlStats *p)
{
    u_char *row;

    if (p->no_growth != 0) {
        return;
    }
    if (p->level <= g_btl_enemy_level + 3) {
        p->exp += 3;
        p->rank_exp += 3;
    } else if (p->level <= g_btl_enemy_level * 2 / 3) {
        p->exp += 2;
        p->rank_exp += 2;
    } else {
        p->exp += 1;
        p->rank_exp += 1;
    }
    p->rank_left = g_btl_persona_rank_exp[p->slots] - p->rank_exp;
    if (p->slots < GROW_RANKS && p->rank_left <= 0) {
        p->slots++;
        p->rank_exp = -p->rank_left;
        p->rank_left = g_btl_persona_rank_exp[p->slots] - p->rank_exp;
        row = g_btl_persona_growth[p->growth];
        p->stat[0] += row[0 * GROW_COLUMNS + p->slots - GROW_FIRST];
        p->stat[1] += row[1 * GROW_COLUMNS + p->slots - GROW_FIRST];
        p->stat[2] += row[2 * GROW_COLUMNS + p->slots - GROW_FIRST];
        p->stat[3] += row[3 * GROW_COLUMNS + p->slots - GROW_FIRST];
        p->stat[4] += row[4 * GROW_COLUMNS + p->slots - GROW_FIRST];
        p->mag_atk += row[5 * GROW_COLUMNS + p->slots - GROW_FIRST];
        p->mag_def += row[6 * GROW_COLUMNS + p->slots - GROW_FIRST];
        p->stat[0] = CHAR_GROW_CLAMP(p->stat[0], GROW_STAT_MAX);
        p->stat[1] = CHAR_GROW_CLAMP(p->stat[1], GROW_STAT_MAX);
        p->stat[2] = CHAR_GROW_CLAMP(p->stat[2], GROW_STAT_MAX);
        p->stat[3] = CHAR_GROW_CLAMP(p->stat[3], GROW_STAT_MAX);
        p->stat[4] = CHAR_GROW_CLAMP(p->stat[4], GROW_STAT_MAX);
        p->mag_atk = CHAR_GROW_CLAMP(p->mag_atk, GROW_NUM_MAX);
        p->mag_def = CHAR_GROW_CLAMP(p->mag_def, GROW_NUM_MAX);
    }
}

/* 98.50%, registers only. Indexing the highest-stat search off `base`, with
   i cleared before best, put the fighter in a2 as in the image. What is left:
   the level read at the top sits in a0 rather than a3, the two clamps swap
   a0/a1, and the stat loop's key product and level swap v0/v1. A local for
   the level changes nothing (CSE already keeps it). Folding the column into
   one index (`rows[key * ROW + half]`) moves the product out of order
   (88.6%). */
#ifdef NON_MATCHING
void BtlDrainLevel(BtlActor *a)
{
    int    *curve;
    u_char *base;
    u_char *stat;
    u_char *rows;
    u_char *growth;
    u_char  best;
    int     which;
    int     row;
    int     sp;
    int     hp;
    int     i;

    curve = g_level_exp_1;
    if (a->c.level != 1) {
        if (a->c.key == DRAIN_HERO) {
            i = 0;
            best = 0;
            base = a->c.stat_base;
            for (; i < CHAR_STATS; i++) {
                if (base[i] >= best) {
                    best = base[i];
                    which = i;
                }
            }
            base[(u_char)which] -= DRAIN_STAT;
            sp = a->c.sp;
            a->c.hp_max -= DRAIN_MAXIMA;
            a->c.sp_max -= DRAIN_MAXIMA;
            if (a->c.sp_max < sp) {
                sp = a->c.sp_max;
            }
            a->c.sp = sp;
            hp = a->c.hp;
            if (a->c.hp_max < hp) {
                hp = a->c.hp_max;
            }
            a->c.hp = hp;
        } else {
            row = a->c.key - DRAIN_FIRST;
            sp = a->c.sp;
            rows = g_char_hp_growth + row * DRAIN_HP_ROW;
            a->c.hp_max -= rows[a->c.level - DRAIN_FIRST];
            a->c.sp_max -= DRAIN_MAXIMA;
            if (a->c.sp_max < sp) {
                sp = a->c.sp_max;
            }
            a->c.sp = sp;
            hp = a->c.hp;
            if (a->c.hp_max < hp) {
                hp = a->c.hp_max;
            }
            a->c.hp = hp;
            i = 0;
            stat = a->c.stat_base;
            rows = g_char_stat_growth;
            do {
                i++;
                growth = rows + a->c.key * DRAIN_STAT_ROW;
                *stat -= growth[(a->c.level - DRAIN_FIRST) / 2];
                rows += DRAIN_HP_ROW;
                stat++;
            } while (i < CHAR_STATS);
        }
        a->c.level--;
    }

    a->c.unk10 = 0;
    a->c.unk14 = 0;
    a->c.unk18 = curve[a->c.level - 1];
    for (i = 0; i < a->c.level - 1; i++) {
        a->c.unk10 += curve[i];
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/personagrow", BtlDrainLevel);
#endif
