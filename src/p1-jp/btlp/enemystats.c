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
#include <types.h>

/* Spell slots an enemy has. */
#define BTL_ENEMY_SPELLS 7

/* A slot that holds nothing. */
#define BTL_SLOT_NONE 0xFF

/* The five stats, in the order the record keeps them. */
#define STAT_STRENGTH  0
#define STAT_VITALITY  1
#define STAT_DEXTERITY 2
#define STAT_AGILITY   3
#define STAT_LUCK      4

typedef struct {
    /* 0x00 */ u_char  pad00[0x18];
    /* 0x18 */ u_short attack;
    /* 0x1A */ u_short accuracy;
    /* 0x1C */ u_short guard;
    /* 0x1E */ u_char  pad1E[0xC];
    /* 0x2A */ u_char  level;
    /* 0x2B */ u_char  pad2B[1];
    /* 0x2C */ u_char  stat[5];
    /* 0x31 */ u_char  pad31[1];
    /* 0x32 */ u_char  slots;                    /* entries of the order to walk */
    /* 0x33 */ u_char  spell[BTL_ENEMY_SPELLS];  /* the packed list             */
    /* 0x3A */ u_char  raw[BTL_ENEMY_SPELLS];    /* as it came off the disc     */
} BtlEnemyStats;

extern const u_char g_btl_spell_order[];

void BtlEnemyDeriveStats(BtlEnemyStats *e)
{
    BtlEnemyStats *dst;
    int i;
    int n;
    int slot;
    /* A local the original declares and never uses; the frame is 8 bytes
       without anything being spilled into it. */
    int spare;

    /* The clear walks the record a byte at a time with the field as the
       displacement rather than walking the array itself - that is the shape
       the original has, and the tidier spelling recomputes the address. */
    i = BTL_ENEMY_SPELLS - 1;
    dst = (BtlEnemyStats *)((u_char *)e + BTL_ENEMY_SPELLS - 1);
    for (; i >= 0; i--) {
        dst->spell[0] = 0;
        dst = (BtlEnemyStats *)((u_char *)dst - 1);
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
