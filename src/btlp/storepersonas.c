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

extern void BtlEnemyDeriveStats(BtlStats *s);

void BtlStorePersonas(void)
{
    BtlStats *s;
    Persona  *d;
    int       i;

    d = g_personas;
    s = g_btl_personas;
    i = 0;
    do {
        d->unk00 = s->unk00;
        d->unk04 = s->unk04;
        d->unk08 = s->unk08;
        d->unk0C = s->unk0C;
        d->unk10 = s->unk10;
        d->unk12 = s->unk12;
        d->key   = s->key;
        memcpy(d->unk19, s->unk1F, 10);
        d->unk23 = s->unk29;
        d->level = s->level;
        d->kind  = s->kind;
        d->stat[0] = s->stat[0];
        d->stat[1] = s->stat[1];
        d->stat[2] = s->stat[2];
        d->stat[3] = s->stat[3];
        d->stat[4] = s->stat[4];
        d->unk2B = s->unk31;
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
            d->unk00 = 0;
            d->unk04 = 0;
            d->unk08 = 0;
            d->unk0C = def->unk1C;
            d->unk10 = def->unk04;
            d->unk12 = def->unk06;
            d->key = key;
            memcpy(d->unk1F, def->unk08, 10);
            d->unk29 = def->unk12;
            d->level = def->level;
            d->kind = def->kind;
            d->stat[0] = def->stat[0];
            d->stat[1] = def->stat[1];
            d->stat[2] = def->stat[2];
            d->stat[3] = def->stat[3];
            d->stat[4] = def->stat[4];
            d->unk31 = def->unk1A;
            d->slots = 1;
            memcpy(d->raw, def->raw, BTL_STATS_SPELLS);
            d->unk41 = def->raw[BTL_STATS_SPELLS - 1];
            d->unk44 = def->unk28;
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
