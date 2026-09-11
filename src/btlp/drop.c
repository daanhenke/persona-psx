/* Persona 1 (JP) - what a battle drops, and how rare it is.
 *   BTLP @ 0x80072D38 BtlPickHoldable, 0x80072E3C BtlPickItemFrom
 *         0x80072F70 BtlRollCommon,   0x80072FA0 BtlRollUncommon
 *         0x80073018 BtlRollDrop
 *
 * Four tiers of candidate item, 0 the rarest and 3 the commonest. The ladder
 * below picks a tier and BtlPickItemFrom walks that tier and every commoner one
 * after it, so a rare roll falls through to a common item rather than to
 * nothing.
 *
 *   BtlRollDrop      3/4 common, else 7/8 to BtlRollUncommon, else tier 0
 *   BtlRollUncommon  7/8 common, else 3/4 tier 2, else tier 1
 *   BtlRollCommon    tier 3
 *
 * Every step reseeds from the frame counter first, so two rolls in the same
 * frame come out the same - which is why the ladder reseeds again at each
 * level rather than drawing several numbers from one seed.
 *
 * A candidate only counts if the party can hold it. That test, and the stock
 * and inventory edits, are a unit of their own well past this one, in
 * dropitems.c.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/offer.h>
#include <persona/common/item.h>

extern int VSync(int mode);
extern int BtlPickItemFrom(unsigned int tier);

/* Keeps the ids from a candidate list that the party can actually hold, then
   picks one of those at random. The kept ids and the slots BtlItemSlot found
   for them share one array - ids in the first half, slots in the second - so
   that a single walking pointer fills both. */
unsigned int BtlPickHoldable(unsigned int count, const u_short *ids)
{
    unsigned int kept[18];
    unsigned int found;
    unsigned int item;
    unsigned int *p;
    const u_short *id;
    int limit;
    int i;
    int n;

    n = 0;
    i = 0;
    count &= 0xFF;
    if ((int)count > 0) {
        /* Save the bound before reusing count for the packed item-slot result. */
        limit = count;
        id = ids;
        p = kept;
        do {
            count = BtlItemSlot(*id);
            if ((count & 0xFFFF) != 0) {
                n++;
                item = *id;
                p[8] = count >> 16;
                *p = item;
                p++;
            }
            i++;
            id++;
        } while (i < limit);
    }
    found = 0;
    if (n != 0) {
        srand(VSync(-1));
        i = rand() % n;
        found = kept[i + 8] << 16 | kept[i];
    }
    return found;
}

/* The candidates, sixteen consecutive item ids split across the four tiers.
   Each row is eight wide however many it uses, and the count beside it says
   how far in the row is filled, so the two are read together. */
#define TIERS    4
#define TIER_MAX 8

/* Walks the named tier and every commoner one after it, taking the first item
   the party can still hold. Returning zero means every tier from here down was
   refused, which is what makes a rare roll fall through to a common item
   rather than to nothing. A tier past the last is not a drop at all. */
int BtlPickItemFrom(unsigned int tier)
{
    u_char   counts[TIERS] = { 2, 3, 3, 8 };
    u_short  ids[TIERS][TIER_MAX] = {
        { 0x56, 0x57 },
        { 0x58, 0x59, 0x5A },
        { 0x5B, 0x5C, 0x5D },
        { 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65 },
    };
    int i;
    int id;

    i = (u_char)tier;
    if (i < TIERS) {
        do {
            id = BtlPickHoldable(counts[i], ids[i]);
            if (id != 0) {
                return id;
            }
            i++;
        } while (i < TIERS);
    }
    return 0;
}

void BtlRollCommon(void)
{
    srand(VSync(-1));
    BtlPickItemFrom(3);
}

void BtlRollUncommon(void)
{
    srand(VSync(-1));
    if ((rand() & 7) == 0) {
        if ((rand() & 3) == 0) {
            BtlPickItemFrom(1);
        } else {
            BtlPickItemFrom(2);
        }
    } else {
        BtlRollCommon();
    }
}

void BtlRollDrop(void)
{
    srand(VSync(-1));
    if ((rand() & 3) == 0) {
        if ((rand() & 7) == 0) {
            BtlPickItemFrom(0);
        } else {
            BtlRollUncommon();
        }
    } else {
        BtlRollCommon();
    }
}
