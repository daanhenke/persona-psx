/* Persona 1 (JP) - putting the equipped Persona's numbers on a member.
 *   0x80085F40 BtlApplyPersona    BTLP only.
 *
 * Only a party member has one, which is what the test on the object's marker
 * number is for. With no Persona equipped, or with the character blocked, the
 * whole block is put back: the two the negotiation reads go to one and the
 * rest to zero.
 *
 * Otherwise the Persona's own numbers are copied across, its five stats become
 * the floor the character's are held at, and its key earns a rank out of a
 * table of thresholds.
 *
 * The seven equipment slots are then totalled a nibble at a time - the same
 * three bonus bytes the field's own CharRecalcStats reads - and each stat
 * becomes the larger of the character's base plus that total and the Persona's
 * own, clamped to 1..99. A stat that lands on zero is lifted to one rather
 * than clamped, which is why the test is written the way it is.
 *
 * The last two lines are what the negotiation weighs a contact with.
 */
#include <decomp/types.h>
#include <persona/common/item.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/stats.h>

/* Members occupy the first five marker numbers. */
#define BTL_MEMBERS 5

/* Stats a character has, and equipment slots. */
#define BTL_STATS 5
#define BTL_EQUIP 7

/* As far as a stat goes either way. */
#define STAT_MAX 99

/* Rows of the rank table. */
#define BTL_PERSONA_RANKS      23

/* What the block goes back to with no Persona. */
#define PERSONA_NONE_NUM 1

extern BtlStats g_btl_personas[];
extern ItemDef  g_item_defs[];

/* One row: the key it covers, then the rank four bytes on. */
typedef struct {
    /* 0x0 */ int    upto;
    /* 0x4 */ u_char rank;
    /* 0x5 */ u_char pad05[3];
} BtlPersonaRank;                 /* 8 bytes */

extern BtlPersonaRank g_btl_persona_ranks[];

/* A debug switch beside the others at 0x8004E260: while it is set, the
   Persona's two numbers are not written back over the fighter's own. */
extern u_char g_btl_debug_keep_numbers;

extern int BtlActorPersona(int slot);

void BtlApplyPersona(BtlActor *a)
{
    BtlStats *p;
    int       which;
    int       n;
    int       i;
    int       rank;
    u_short  *e;
    int       item;

    if (a->obj->mark_num < BTL_MEMBERS) {
        which = BtlActorPersona(a->obj->mark_num);
        if (which >= 0 && a->c.blocked == 0) {
            p = &g_btl_personas[which];
            a->persona_num[0] = p->mag_atk;
            a->persona_num[1] = p->mag_def;
            a->persona_num[2] = p->unk14;
            a->persona_num[3] = p->unk16;
            a->persona_sum = p->stat[2] * 2 + p->stat[3] + p->stat[4] / 2;
            a->persona_stat[0] = p->stat[0];
            a->persona_stat[1] = p->stat[1];
            a->persona_stat[2] = p->stat[2];
            a->persona_stat[3] = p->stat[3];
            a->persona_stat[4] = p->stat[4];
            a->c.resist = p->resist;
            for (rank = 0; rank < BTL_PERSONA_RANKS; rank++) {
                if (p->key <= g_btl_persona_ranks[rank].upto) {
                    a->persona_rank = g_btl_persona_ranks[rank].rank;
                    break;
                }
            }
        } else {
            a->persona_num[0] = PERSONA_NONE_NUM;
            a->persona_num[1] = PERSONA_NONE_NUM;
            a->persona_num[2] = 0;
            a->persona_num[3] = 0;
            a->persona_sum = 0;
            a->persona_stat[0] = 0;
            a->persona_stat[1] = 0;
            a->persona_stat[2] = 0;
            a->persona_stat[3] = 0;
            a->persona_stat[4] = 0;
            a->c.resist = 0;
            a->persona_rank = 0;
        }

        i = 0;
        a->equip_stat[0] = 0;
        a->equip_stat[1] = 0;
        a->equip_stat[2] = 0;
        a->equip_stat[3] = 0;
        a->equip_stat[4] = 0;
        /* The slot is walked with a pointer but the table is indexed afresh
           for every bonus - a pointer to the record folds the five accesses
           together, which the original does not do. */
        e = a->c.equip;
        do {
            item = *e;
            a->equip_stat[0] += g_item_defs[item].bonus01 >> 4;
            a->equip_stat[1] += g_item_defs[item].bonus01 & 0xF;
            a->equip_stat[2] += g_item_defs[item].bonus23 >> 4;
            a->equip_stat[3] += g_item_defs[item].bonus23 & 0xF;
            a->equip_stat[4] += g_item_defs[item].bonus4 >> 4;
            e++;
            i++;
        } while (i < BTL_EQUIP);

        n = a->c.stat_base[0] + a->equip_stat[0];
        if (n < a->persona_stat[0]) {
            n = a->persona_stat[0];
        }
        a->c.stat[0] = n;
        n = a->c.stat_base[1] + a->equip_stat[1];
        if (n < a->persona_stat[1]) {
            n = a->persona_stat[1];
        }
        a->c.stat[1] = n;
        n = a->c.stat_base[2] + a->equip_stat[2];
        if (n < a->persona_stat[2]) {
            n = a->persona_stat[2];
        }
        a->c.stat[2] = n;
        n = a->c.stat_base[3] + a->equip_stat[3];
        if (n < a->persona_stat[3]) {
            n = a->persona_stat[3];
        }
        a->c.stat[3] = n;
        n = a->c.stat_base[4] + a->equip_stat[4];
        if (n < a->persona_stat[4]) {
            n = a->persona_stat[4];
        }
        a->c.stat[4] = n;

        a->c.stat[0] = CHAR_GROW_CLAMP(a->c.stat[0], STAT_MAX);
        a->c.stat[1] = CHAR_GROW_CLAMP(a->c.stat[1], STAT_MAX);
        a->c.stat[2] = CHAR_GROW_CLAMP(a->c.stat[2], STAT_MAX);
        a->c.stat[3] = CHAR_GROW_CLAMP(a->c.stat[3], STAT_MAX);
        a->c.stat[4] = CHAR_GROW_CLAMP(a->c.stat[4], STAT_MAX);

        if (g_btl_debug_keep_numbers == 0) {
            a->c.mag_atk = a->persona_num[0];
            a->c.mag_def = a->persona_num[1];
        }
    }
}

