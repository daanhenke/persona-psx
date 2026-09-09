/* Persona 1 (JP) - filling a fighter's record from a Persona's.  BTLP only.
 *   0x80087848 BtlLoadEnemyStats
 *
 * Every demon on the field is a Persona underneath, so a slot is set up by
 * copying the reference record out of g_persona_data and deriving the rest.
 * The key it is given is the row it was taken from, which is what lets the
 * negotiation look the same record up again later.
 *
 * Health and spirit are written twice over, once as the current value and once
 * as the maximum, so a demon always arrives whole. The six fighting numbers
 * are worked out here rather than copied: each is a weighted sum of three of
 * the five stats, taken to one and a half by multiplying by 150 and dividing
 * by 100 - which is the arithmetic the original spells out, not a shift.
 *
 * Everything the record cannot supply is cleared, and BtlDeriveBattleStats
 * finishes the job by bending the numbers for whatever the fighter is under.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>

/* Bytes of the name the record carries over. */
#define ENEMY_NAME_BYTES 10

/* The six numbers are taken to one and a half this way round; written as a
   shift or as * 3 / 2 it is not the same code. */
#define ENEMY_SCALE_NUM 150
#define ENEMY_SCALE_DEN 100

/* Which of the five rows a demon's arcana earns. The table is 1-based, so
   entry nought is never read, and the three past the last arcana are the
   alignment the assembler needs rather than entries of their own. */
u_char g_btl_arcana_rank[24] = {
    0x00, 0x03, 0x01, 0x01, 0x04, 0x03, 0x04, 0x02,
    0x01, 0x02, 0x04, 0x03, 0x02, 0x04, 0x01, 0x01,
    0x03, 0x03, 0x02, 0x02, 0x03, 0x00, 0x00, 0x00,
};

extern void BtlDeriveBattleStats(BtlActor *a);

void BtlLoadEnemyStats(int slot, int key)
{
    BtlActor          *a;
    const PersonaData *d;

    a = &g_btl_combatants[slot];
    d = &g_persona_data[key];

    /* Both halves of each pair come off the same word, and the record is read
       again for the second: folding them into one load is not what the
       original does. */
    a->c.hp = d->hp;
    a->c.sp = d->sp;
    a->c.hp_max = d->hp;
    a->c.sp_max = d->sp;
    a->c.unk10 = d->exp;
    a->c.unk3A = d->unk0C;
    a->c.unk3C = d->unk0E;
    *(int *)a->c.pad1C = d->unk10;
    a->unk7C = d->unk14;
    a->unk7E = (u_short)d->price;

    /* The key is the row it was taken from. */
    a->c.key = key;
    a->c.status = 0;
    a->c.ail_level = 0;
    a->flags = 0;
    a->c.level = d->level;
    a->species = d->arcana;
    a->persona_rank = g_btl_arcana_rank[d->arcana];
    a->c.stat[STAT_STRENGTH] = d->stat[STAT_STRENGTH];
    a->c.stat[STAT_VITALITY] = d->stat[STAT_VITALITY];
    a->c.stat[STAT_DEXTERITY] = d->stat[STAT_DEXTERITY];
    a->c.stat[STAT_AGILITY] = d->stat[STAT_AGILITY];
    a->c.stat[STAT_LUCK] = d->stat[STAT_LUCK];
    a->c.unk5C = d->unk2D;
    a->c.gun_atk = 0;
    a->c.gun_hit = 0;

    a->c.melee_atk = (d->stat[STAT_STRENGTH] + d->stat[STAT_DEXTERITY] / 2
                      + d->level / 5) * ENEMY_SCALE_NUM / ENEMY_SCALE_DEN;
    a->c.melee_hit = (d->stat[STAT_DEXTERITY] + d->stat[STAT_AGILITY] / 2
                      + d->stat[STAT_LUCK] / 4) * ENEMY_SCALE_NUM
                     / ENEMY_SCALE_DEN;
    a->c.defence = (d->stat[STAT_VITALITY] + d->stat[STAT_AGILITY] / 2
                    + d->level / 5) * ENEMY_SCALE_NUM / ENEMY_SCALE_DEN;
    a->c.evade = (d->stat[STAT_AGILITY] + d->stat[STAT_DEXTERITY] / 2
                  + d->stat[STAT_LUCK] / 4) * ENEMY_SCALE_NUM / ENEMY_SCALE_DEN;

    memcpy(a->c.name, d->name, ENEMY_NAME_BYTES);
    memcpy(a->unkAF, d->unk2E, sizeof(a->unkAF));

    a->unk80 = 0;
    a->unk84 = 0;
    a->pickable = 0;
    a->unkCC = 0;
    a->offered = 0;
    a->unkE1[0] = 0;
    a->unkE1[1] = 0;
    a->unkE1[2] = 0;
    a->unkE1[3] = 0;
    a->unkE1[4] = 0;
    a->unkE1[5] = 0;
    a->unkE1[6] = 0;
    a->unkD2 = 0;
    a->unkD3 = 0;
    a->unkD4 = 0;
    a->unkDF = 0;
    a->unkC5 = 0;
    a->unkC6 = 0;
    BtlDeriveBattleStats(a);
}
