/* Persona 1 (JP) - the party's Personas, brought into the battle.  BTLP only.
 *   0x80086A90 BtlLoadPersonas
 *
 * The save game keeps thirty-one Persona records of 0x40 bytes; the battle
 * fights with records of its own that are 0x48. The two layouts agree up to
 * +0x13, the battle's then has ten bytes of its own, and everything from the
 * save record's key at +0x18 lands ten bytes further along.
 *
 * Two things the save game does not hold are filled in here: the byte the
 * definition record keeps at +0x28, looked up through the key that was just
 * copied, and the three numbers BtlEnemyDeriveStats works out - the same call
 * that finishes an enemy read off the disc, which is what says the party's
 * records and the enemies' are one type.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/common/persona.h>
#include <persona/btlp/stats.h>

extern void BtlEnemyDeriveStats(BtlStats *s);

void BtlLoadPersonas(void)
{
    const PersonaDef *def;
    BtlStats *d;
    Persona  *s;
    int       i;

    s = g_personas;
    d = g_btl_personas;
    i = 0;
    do {
        d->unk00 = s->unk00;
        d->unk04 = s->unk04;
        d->unk08 = s->unk08;
        d->unk0C = s->unk0C;
        d->unk10 = s->unk10;
        d->unk12 = s->unk12;
        d->key   = s->key;
        memcpy(d->unk1F, s->unk19, 10);
        d->unk29 = s->unk23;
        d->level = s->level;
        d->kind  = s->kind;
        d->stat[0] = s->stat[0];
        d->stat[1] = s->stat[1];
        d->stat[2] = s->stat[2];
        d->stat[3] = s->stat[3];
        d->stat[4] = s->stat[4];
        d->unk31 = s->unk2B;
        d->slots = s->slots;
        memcpy(d->spell, s->spell, BTL_STATS_SPELLS);
        memcpy(d->raw, s->raw, BTL_STATS_SPELLS);
        d->unk41 = s->unk3B;
        /* The definition is reached through a pointer of its own; indexing
           g_persona_defs at the point of use scales the key twice. */
        def = &g_persona_defs[d->key];
        d->unk44 = def->unk28;
        d->unk43 = 0;
        BtlEnemyDeriveStats(d);
        i++;
        s++;
        d++;
    } while (i < PERSONA_COUNT);
}
