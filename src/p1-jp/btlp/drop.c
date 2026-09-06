/* Persona 1 (JP) - what a battle drops, and how rare it is.
 *   BTLP @ 0x80074018 BtlItemSlot,   0x80072F70 BtlRollCommon
 *         0x80072FA0 BtlRollUncommon, 0x80073018 BtlRollDrop
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
 * A candidate only counts if the party can hold it. BtlItemSlot is that test:
 * the inventory packs an id into the low nine bits of a slot and the count
 * above them, the same way a chest's payload does.
 */
#include <types.h>
#include <persona/btlp/offer.h>

#define ITEM_SLOTS 0x17E
#define ITEM_ID    0x1FF
#define ITEM_SHIFT 9

/* The count occupies the seven bits above the id. */
#define ITEM_COUNT 0x7F

/* Held this many or more and the item is refused. */
#define ITEM_MAX 0x63

/* The low half of what BtlItemSlot returns. */
#define SLOT_FULL  0
#define SLOT_EMPTY 1
#define SLOT_SOME  2

/* The Persona stock, and the twelve of its fifteen slots the screen shows. */
#define g_persona_stock ((u_char *)0x801F297C)
#define STOCK_ROWS 12
#define STOCK_FREE 0

/* Reached by hardcoded address here rather than through the linker symbol. */
#define g_items ((u_short *)0x801F267C)

extern int VSync(int mode);
extern void srand(unsigned int seed);
extern int rand(void);
extern int BtlPickItemFrom(unsigned int tier);

/* The slot in the high half, and what is in it in the low. Falling off the end
   of the search restarts it looking for an empty slot instead. */
unsigned int BtlItemSlot(u_short id)
{
    u_short     *p;
    int          i;
    unsigned int slot;

    p = g_items;
    i = 0;
    id &= 0xFFFF;
    slot = 0;
    for (;;) {
        if ((*p & ITEM_ID) == id) {
            if (*p >> ITEM_SHIFT < ITEM_MAX) {
                if (*p >> ITEM_SHIFT != 0) {
                    return slot | SLOT_SOME;
                }
                return slot | SLOT_EMPTY;
            }
            return slot;
        }
        slot += 0x10000;
        i++;
        p++;
        if (i >= ITEM_SLOTS) {
            break;
        }
    }
    p = g_items;
    i = 0;
    do {
        if ((*p & ITEM_ID) == 0) {
            return i << 16 | SLOT_EMPTY;
        }
        i++;
        p++;
    } while (i < ITEM_SLOTS);
    return 0;
}

/* Is there anywhere to put a Persona the battle has just been given? Twelve
   slots, not the fifteen the array holds - twelve is what the stock screen
   draws, and what the compaction in common/game/personastock.c tidies. */
int BtlStockHasRoom(void)
{
    u_char *stock;
    int     i;
    int     room;

    stock = g_persona_stock;
    i = 0;
    do {
        if (*stock == STOCK_FREE) {
            room = 1;
            goto done;
        }
        i++;
        stock++;
    } while (i < STOCK_ROWS);
    room = 0;
done:
    return room;
}

/* Is the party already carrying the Persona this offer would hand over? Same
   twelve slots BtlStockHasRoom looks at. */
int BtlStockHolds(const BtlOffer *offer)
{
    u_char *stock;
    int     i;
    int     held;

    stock = g_persona_stock;
    i = 0;
    do {
        if (*stock == offer->persona) {
            held = 1;
            goto done;
        }
        i++;
        stock++;
    } while (i < STOCK_ROWS);
    held = 0;
done:
    return held;
}

/* One more of an item. BtlItemSlot returning SLOT_FULL in its low half is the
   refusal, so anything else means there is room. The count is seven bits wide
   and stops at ITEM_MAX. */
int BtlItemAdd(int id)
{
    u_short     *p;
    unsigned int slot;
    int          n;

    p = g_items;
    slot = BtlItemSlot(id);
    if ((slot & 0xFFFF) != SLOT_FULL) {
        p += slot >> 16;
        n = *p >> ITEM_SHIFT;
        n = n + 1;
        if (n > ITEM_MAX) {
            n = ITEM_MAX;
        }
        *p = id | (n & ITEM_COUNT) << ITEM_SHIFT;
    }
}

/* One fewer, and the slot goes back to nothing when the last one goes. An
   empty slot means the party was not carrying any, so there is nothing to do. */
int BtlItemRemove(int id)
{
    u_short     *p;
    unsigned int slot;
    int          left;

    p = g_items;
    slot = BtlItemSlot(id);
    if ((slot & 0xFFFF) != SLOT_EMPTY) {
        p += slot >> 16;
        left = (*p >> ITEM_SHIFT) - 1;
        if (left <= 0) {
            *p = 0;
        } else {
            *p = id | (left & ITEM_COUNT) << ITEM_SHIFT;
        }
    }
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
