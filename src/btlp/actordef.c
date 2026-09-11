/* Persona 1 (JP) - a fighter's numbers from the Persona behind it.  BTLP only.
 *   0x800B5BC0 BtlActorFromDef
 *
 * The same copy BtlLoadEnemyStats makes, less everything that only applies to
 * a demon arriving on the field: no health or spirit, no key, and nothing is
 * cleared. It is what a record already in the fight is brought back into line
 * with when the Persona behind it changes.
 *
 * The four fighting numbers are worked out rather than copied, each a weighted
 * sum of three of the five stats scaled by a fifth. The enemy loader spells
 * the same four out at three halves - a demon of the same Persona is stronger
 * than a member carrying it.
 *
 * @bug The five stats are not copied to one place. Strength and vitality go on
 * to Char's own five, which is what the status screen and the field read, and
 * the other three to the fight's copy at +0xA0, which is what BtlDeriveBattleStats
 * bends and everything in the battle reads. Whichever of the two the caller
 * meant, three of the five land in the wrong one.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>

/* Bytes of the name the record carries over. */
#define ACTOR_NAME_BYTES 10

/* The four numbers are taken to a fifth more this way round, the way the
   enemy loader takes its own to a half more. */
#define ACTOR_SCALE_NUM 120
#define ACTOR_SCALE_DEN 100

/* How much of the level counts towards a fighting number. */
#define ACTOR_LEVEL_DIV 5

extern void BtlDeriveBattleStats(BtlActor *a);

void BtlActorFromDef(BtlActor *a, int key)
{
    const PersonaData *d;

    d = &g_persona_data[key];

    a->c.unk10 = d->exp;
    a->c.unk3A = d->unk0C;
    a->c.unk3C = d->unk0E;
    *(int *)a->c.pad1C = d->unk10;
    a->drop = d->drop;
    a->price = (u_short)d->price;

    a->c.level = d->level;
    a->species = d->arcana;
    a->c.stat[STAT_STRENGTH] = d->stat[STAT_STRENGTH];
    a->c.stat[STAT_VITALITY] = d->stat[STAT_VITALITY];
    a->stat[STAT_DEXTERITY] = d->stat[STAT_DEXTERITY];
    a->stat[STAT_AGILITY] = d->stat[STAT_AGILITY];
    a->stat[STAT_LUCK] = d->stat[STAT_LUCK];
    a->c.unk5C = d->unk2D;

    a->c.melee_atk = (d->stat[STAT_STRENGTH] + d->stat[STAT_DEXTERITY] / 2
                      + d->level / ACTOR_LEVEL_DIV) * ACTOR_SCALE_NUM
                     / ACTOR_SCALE_DEN;
    a->c.melee_hit = (d->stat[STAT_DEXTERITY] + d->stat[STAT_AGILITY] / 2
                      + d->stat[STAT_LUCK] / 4) * ACTOR_SCALE_NUM
                     / ACTOR_SCALE_DEN;
    a->c.defence = (d->stat[STAT_VITALITY] + d->stat[STAT_AGILITY] / 2
                    + d->level / ACTOR_LEVEL_DIV) * ACTOR_SCALE_NUM
                   / ACTOR_SCALE_DEN;
    a->c.evade = (d->stat[STAT_AGILITY] + d->stat[STAT_DEXTERITY] / 2
                  + d->stat[STAT_LUCK] / 4) * ACTOR_SCALE_NUM
                 / ACTOR_SCALE_DEN;

    memcpy(a->c.name, d->name, ACTOR_NAME_BYTES);
    memcpy(a->spell, d->spell, sizeof(a->spell));
    BtlDeriveBattleStats(a);
}
