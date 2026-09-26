/* Persona 1 (JP) - using an item on a party member outside battle.
 *
 * Compiled into two overlays rather than called across the boundary:
 *   ADV 0x80094074   DNG 0x8008E948
 *
 * Recovery items go through SpellApplyField as the spell they work like;
 * the rest act on the member's record here: a base stat or a maximum raised,
 * HP or SP topped up, an ailment cured - and one tonic that heals past the
 * member's limit poisons them for it. Whatever the item, the member's stats
 * are worked out again afterwards. Defined old-style: both numbers are
 * narrowed here.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

/* The flag the escape spell raises, reached by address. */
#define FIELD_SPELL (*(u_char *)0x801F29A8)

#define CAP_MAX 999

extern int  rand(void);
extern void SpellApplyField();
#ifdef ITEMUSE_NOPROTO
/* DNG's build saw these without prototypes, so the member's index goes to
   both as the int it was loaded as and stays in one register across the two
   calls. */
extern void CharApplyStats();
extern void CharRecalcStats();
#else
extern void CharApplyStats(u_char chr);
extern void CharRecalcStats(u_char chr);
#endif

void ItemUseOn(member, item)
    short member;
    short item;
{
    int   n = g_party[member];   /* the member's record, then again below */
    Char *c = &g_chars[n];

    switch (item) {
    case 0x01:
    case 0x10:
        SpellApplyField(member, member, 0x5F);
        break;
    case 0x02:
        SpellApplyField(member, member, 0x60);
        break;
    case 0x03:
        SpellApplyField(member, member, 0x61);
        break;
    case 0x08:
    case 0x09:
        SpellApplyField(member, member, 0x67);
        break;
    case 0x12:
        SpellApplyField(0xFF, 0xFF, 0x73);
        FIELD_SPELL = 0;
        break;
    case 0x13:
        c->stat_base[0]++;
        goto refill;
    case 0x14:
        c->stat_base[1]++;
        goto refill;
    case 0x15:
        c->stat_base[2]++;
        goto refill;
    case 0x16:
        c->stat_base[3]++;
        goto refill;
    case 0x17:
        c->stat_base[4]++;
        goto refill;
    case 0x18:
        c->hp_max += 5;
        if (c->hp_max > CAP_MAX) {
            c->hp_max = CAP_MAX;
        }
    refill:
        c->hp = c->hp_max;
        break;
    case 0x19:
        c->sp_max += 5;
        if (c->sp_max > CAP_MAX) {
            c->sp_max = CAP_MAX;
        }
        c->sp = c->sp_max;
        break;
    case 0x1A:
        c->hp += (c->level >> 1) + (rand() & 7);
        if (c->hp > CAP_MAX) {
            c->hp = CAP_MAX;
        }
        if (c->hp > c->hp_max * 3 / 2) {
            c->hp = c->hp_max / 2;
            goto poison;
        }
        if (rand() % ((c->stat[4] >> 1) + 1) == 0) {
            c->status = STATUS_POISON;
            c->hp = c->hp_max / 2;
        }
        break;
    case 0x1B:
        c->hp += c->hp_max / 8 + (rand() & 7);
        if (c->hp > c->hp_max) {
            c->hp = c->hp_max;
        }
        c->sp += (c->level >> 2) + (rand() & 7) * 2;
        if (c->sp > c->sp_max) {
            c->sp = c->sp_max;
        }
        break;
    case 0x1C:
    full:
        c->hp = c->hp_max;
        c->sp = c->sp_max;
        break;
    case 0x1D:
        c->status = STATUS_GOOD;
        goto full;
    case 0x22:
        c->sp += c->sp_max / 4;
        if (c->sp > c->sp_max) {
            c->sp = c->sp_max;
        }
        break;
    case 0x67:
        SpellApplyField(member, member, 0x63);
        break;
    case 0x68:
    poison:
        c->status = STATUS_POISON;
        break;
    }
    n = g_party[member];
    CharApplyStats(n);
    CharRecalcStats(n);
}
