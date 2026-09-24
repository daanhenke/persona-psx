/* Persona 1 (JP) - a new Persona record.  ADV only.
 *   0x800B053C PersonaCreate
 *
 * Script command 4F gives a character a new Persona: a free g_personas slot
 * is filled out of the Persona's definition, with only the first of its
 * spells learned.
 */
#include <decomp/types.h>
#include <persona/common/persona.h>

#define NO_OWNER 0xFF

void PersonaCreate(u_char slot, u_char id)
{
    Persona          *p;
    const PersonaDef *d;
    u_char            i;

    p = &g_personas[slot];
    d = &g_persona_defs[id];
    p->unk00 = d->unk00;
    p->unk04 = 0;
    p->unk08 = 0;
    p->unk10 = d->unk04;
    p->unk12 = d->unk06;
    p->unk14 = 0;
    p->unk16 = 0;
    p->key = id;
    for (i = 0; i < 10; i++) {
        p->name[i] = d->name[i];
    }
    p->sp_cost = d->sp_cost;
    p->level = d->level;
    p->kind = d->kind;
    p->stat[0] = d->stat[0];
    p->stat[1] = d->stat[1];
    p->stat[2] = d->stat[2];
    p->stat[3] = d->stat[3];
    p->stat[4] = d->stat[4];
    p->resist = d->resist;
    p->slots = 1;
    p->bond = d->bond;
    p->raw[0] = d->raw[0];
    p->raw[1] = d->raw[1];
    p->raw[2] = d->raw[2];
    p->raw[3] = d->raw[3];
    p->raw[4] = d->raw[4];
    p->raw[5] = d->raw[5];
    p->raw[6] = 0;
    p->spell[0] = p->raw[0];
    p->spell[1] = 0;
    p->spell[2] = 0;
    p->spell[3] = 0;
    p->spell[4] = 0;
    p->spell[5] = 0;
    p->spell[6] = 0;
    p->unk3B = d->raw[6];
    p->unk3C = 0;
    p->unk3D = 0;
    p->owner = NO_OWNER;
}
