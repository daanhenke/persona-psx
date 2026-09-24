/* Persona 1 (JP) - the party's Personas, carried back out.  BTLP only.
 *   0x80086C84 BtlStorePersonas
 *
 * The other end of BtlLoadPersonas: the battle's thirty-one 0x48-byte records
 * are written back over the save game's 0x40-byte ones, field for field, the
 * two layouts agreeing up to +0x13 and everything from the key at +0x18
 * landing six bytes earlier here than it sits in the battle's.
 *
 * Nothing the battle worked out for itself goes back - the three numbers
 * BtlEnemyDeriveStats derives, the definition byte kept beside them - because
 * the save game holds none of them and they are worked out again on the way in.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/common/persona.h>
#include <persona/btlp/stats.h>
#include <persona/common/char.h>

void BtlStorePersonas(void)
{
    BtlStats *s;
    Persona  *d;
    int       i;

    d = g_personas;
    s = g_btl_personas;
    i = 0;
    do {
        d->exp = s->exp;
        d->rank_exp = s->rank_exp;
        d->rank_left = s->rank_left;
        d->bond = s->bond;
        d->mag_atk = s->mag_atk;
        d->mag_def = s->mag_def;
        d->key   = s->key;
        memcpy(d->name, s->name, 10);
        d->sp_cost = s->sp_cost;
        d->level = s->level;
        d->kind  = s->kind;
        d->stat[0] = s->stat[0];
        d->stat[1] = s->stat[1];
        d->stat[2] = s->stat[2];
        d->stat[3] = s->stat[3];
        d->stat[4] = s->stat[4];
        d->resist = s->resist;
        d->slots = s->slots;
        memcpy(d->spell, s->spell, PERSONA_SPELLS);
        memcpy(d->raw, s->raw, PERSONA_SPELLS);
        d->unk3B = s->unk41;
        i++;
        s++;
        d++;
    } while (i < PERSONA_COUNT);
}


/* Giving a character a Persona: the first record nobody holds is filled in
   from the definition and the character's list is pointed at it, with the two
   other slots emptied. Nothing to spare means nothing happens.
 *   0x80086E08 BtlGivePersona
 */
void BtlGivePersona(Char *c, int key)
{
    const PersonaDef *def;
    BtlStats         *d;
    int               i;

    d = g_btl_personas;
    def = &g_persona_defs[key];
    i = 0;
    do {
        if (d->key == 0) {
            d->exp = 0;
            d->rank_exp = 0;
            d->rank_left = 0;
            d->bond = def->bond;
            d->mag_atk = def->mag_atk;
            d->mag_def = def->mag_def;
            d->key = key;
            memcpy(d->name, def->name, 10);
            d->sp_cost = def->sp_cost;
            d->level = def->level;
            d->kind = def->kind;
            d->stat[0] = def->stat[0];
            d->stat[1] = def->stat[1];
            d->stat[2] = def->stat[2];
            d->stat[3] = def->stat[3];
            d->stat[4] = def->stat[4];
            d->resist = def->resist;
            d->slots = 1;
            memcpy(d->raw, def->raw, BTL_STATS_SPELLS);
            d->unk41 = def->raw[BTL_STATS_SPELLS - 1];
            d->growth = def->unk28;
            c->entry = 0;
            c->list[0] = i;
            c->list[1] = BTL_SLOT_NONE;
            c->list[2] = BTL_SLOT_NONE;
            BtlEnemyDeriveStats(d);
            return;
        }
        i++;
        d++;
    } while (i < PERSONA_COUNT);
}
