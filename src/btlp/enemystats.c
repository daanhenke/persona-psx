/* Persona 1 (JP) - finishing an enemy's record.  BTLP only.
 *   0x80086FA0 BtlEnemyDeriveStats
 *
 * The data on the disc leaves an enemy half built: its spell slots are laid
 * out in a fixed order with gaps, and the numbers the battle actually fights
 * with are not stored at all. This does both.
 *
 * The spells are packed - read in the order g_btl_spell_order gives, empty
 * slots dropped - so whatever the data looked like, the list the battle reads
 * is contiguous and ends where it ends.
 *
 * The three derived numbers are built the same way the party's are: from a
 * stat, half of a second, and a fraction of a third, all doubled. Strength
 * with dexterity and level makes the attack; dexterity with agility and luck
 * the accuracy; vitality with agility and level the guard.
 */
#include <decomp/types.h>

#include <persona/btlp/stats.h>

extern const u_char g_btl_spell_order[];

void BtlEnemyDeriveStats(BtlStats *e)
{
    BtlStats *dst;
    int i;
    int n;
    int slot;
    /* A local the original declares and never uses; the frame is 8 bytes
       without anything being spilled into it. */
    int spare;

    /* The clear walks the record a byte at a time with the field as the
       displacement rather than walking the array itself - that is the shape
       the original has, and the tidier spelling recomputes the address. */
    i = BTL_STATS_SPELLS - 1;
    dst = (BtlStats *)((u_char *)e + BTL_STATS_SPELLS - 1);
    for (; i >= 0; i--) {
        dst->spell[0] = 0;
        dst = (BtlStats *)((u_char *)dst - 1);
    }

    n = 0;
    for (i = 0; i < (int)e->slots; i++) {
        slot = g_btl_spell_order[i];
        if (slot != BTL_SLOT_NONE) {
            if (e->raw[slot] != 0) {
                e->spell[n] = e->raw[slot];
                n++;
            }
        }
    }

    e->accuracy = (e->stat[STAT_DEXTERITY] + e->stat[STAT_AGILITY] / 2
                   + e->stat[STAT_LUCK] / 4) * 2;
    e->attack = (e->stat[STAT_STRENGTH] + e->stat[STAT_DEXTERITY] / 2
                 + e->level / 5) * 2;
    e->guard = (e->stat[STAT_VITALITY] + e->stat[STAT_AGILITY] / 2
                + e->level / 5) * 2;
}
