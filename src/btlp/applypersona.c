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
#include <decomp/include_asm.h>
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
#define STAT_MIN 1
#define STAT_MAX 99

/* Rows of the rank table. */
#define BTL_PERSONA_RANKS      23
#define BTL_PERSONA_RANK_BYTES 0xB8

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

extern int BtlActorPersona(int slot);

#ifdef NON_MATCHING
void BtlApplyPersona(BtlActor *a)
{
    BtlStats *p;
    int       which;
    int       n;
    int       i;
    int       off;
    int       key;
    u_char    b;
    u_short  *e;
    int       item;

    if (a->obj->unkD2 < BTL_MEMBERS) {
        which = BtlActorPersona(a->obj->unkD2);
        if (which >= 0 && a->c.blocked == 0) {
            p = &g_btl_personas[which];
            a->persona_num[0] = p->unk10;
            a->persona_num[1] = p->unk12;
            a->persona_num[2] = p->unk14;
            a->persona_num[3] = p->unk16;
            a->persona_sum = p->stat[2] * 2 + p->stat[3] + p->stat[4] / 2;
            a->persona_stat[0] = p->stat[0];
            a->persona_stat[1] = p->stat[1];
            a->persona_stat[2] = p->stat[2];
            a->persona_stat[3] = p->stat[3];
            a->persona_stat[4] = p->stat[4];
            off = 0;
            a->c.unk5C = p->unk31;
            key = p->key;
            /* Walked by byte offset against a plain constant: an index would be
               scaled each time round, and sizeof would make the bound
               unsigned. */
            while (off < BTL_PERSONA_RANK_BYTES) {
                if (key <= *(int *)((u_char *)g_btl_persona_ranks + off)) {
                    a->persona_rank = *((u_char *)g_btl_persona_ranks + off + 4);
                    break;
                }
                off += 8;
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
            a->c.unk5C = 0;
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

        /* Written out rather than looped; the original has it unrolled. */
        if (a->c.stat[0] == 0) {
            b = STAT_MIN;
        } else {
            b = a->c.stat[0];
            if (b > STAT_MAX) {
                b = STAT_MAX;
            }
        }
        a->c.stat[0] = b;
        if (a->c.stat[1] == 0) {
            b = STAT_MIN;
        } else {
            b = a->c.stat[1];
            if (b > STAT_MAX) {
                b = STAT_MAX;
            }
        }
        a->c.stat[1] = b;
        if (a->c.stat[2] == 0) {
            b = STAT_MIN;
        } else {
            b = a->c.stat[2];
            if (b > STAT_MAX) {
                b = STAT_MAX;
            }
        }
        a->c.stat[2] = b;
        if (a->c.stat[3] == 0) {
            b = STAT_MIN;
        } else {
            b = a->c.stat[3];
            if (b > STAT_MAX) {
                b = STAT_MAX;
            }
        }
        a->c.stat[3] = b;
        if (a->c.stat[4] == 0) {
            b = STAT_MIN;
        } else {
            b = a->c.stat[4];
            if (b > STAT_MAX) {
                b = STAT_MAX;
            }
        }
        a->c.stat[4] = b;

        a->c.unk3A = a->persona_num[0];
        a->c.unk3C = a->persona_num[1];
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/applypersona", BtlApplyPersona);
#endif

