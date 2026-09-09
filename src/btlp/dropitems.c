/* Persona 1 (JP) - can the party hold this, and taking it.
 *   BTLP @ 0x80073E90 BtlStockHasRoom, 0x80073EC8 BtlStockHolds
 *         0x80073F04 BtlItemAdd,      0x80073F8C BtlItemRemove
 *         0x80074018 BtlItemSlot
 *
 * A unit of its own, well past the drop ladder in drop.c that uses it.
 *
 * BtlItemSlot is the test: the inventory packs an id into the low nine bits of
 * a slot and the count above them, the same way a chest's payload does. It does
 * not come out of the C yet, so the overlay takes it from asm - but it is
 * declared below anyway. The narrowing that prototype forces on the id is a
 * real instruction at each call; without it both callers come out a word
 * short. A cast at each call does the same job, but the declaration says it
 * once and keeps any later caller right.
 */
#include <decomp/types.h>
#include <persona/btlp/offer.h>
#include <persona/common/persona.h>
#include <persona/common/item.h>

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
        *p = id | (n & ITEM_COUNT_MASK) << ITEM_SHIFT;
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
            *p = id | (left & ITEM_COUNT_MASK) << ITEM_SHIFT;
        }
    }
}

/* The slot in the high half, and what is in it in the low. Falling off the end
   of the search restarts it looking for an empty slot instead, and falling off
   that one answers zero without saying so - gcc leaves the loop's own test in
   the return register, which is what the last instruction of the original is
   doing. */
unsigned int BtlItemSlot(u_short id)
{
    u_short *p;
    int      i;

    p = g_items;
    i = 0;
    do {
        if ((*p & ITEM_ID) == id) {
            if (*p >> ITEM_SHIFT < ITEM_MAX) {
                if (*p >> ITEM_SHIFT != 0) {
                    return i << 16 | SLOT_SOME;
                }
                return i << 16 | SLOT_EMPTY;
            }
            return i << 16;
        }
        i++;
        p++;
    } while (i < ITEM_SEARCH);

    p = g_items;
    i = 0;
    do {
        if ((*p & ITEM_ID) == 0) {
            return i << 16 | SLOT_EMPTY;
        }
        i++;
        p++;
    } while (i < ITEM_SEARCH);
}

/* The same three again over the staged list - see g_btl_usable_items in item.h.
   Nothing calls the add or the remove; the search answers only to them. */
int BtlUsableItemAdd(int id)
{
    u_short     *p;
    unsigned int slot;
    int          n;

    p = g_btl_usable_items;
    slot = BtlUsableItemSlot(id);
    if ((slot & 0xFFFF) != SLOT_FULL) {
        p += slot >> 16;
        n = *p >> ITEM_SHIFT;
        n = n + 1;
        if (n > ITEM_MAX) {
            n = ITEM_MAX;
        }
        *p = id | (n & ITEM_COUNT_MASK) << ITEM_SHIFT;
    }
}

int BtlUsableItemRemove(int id)
{
    u_short     *p;
    unsigned int slot;
    int          left;

    p = g_btl_usable_items;
    slot = BtlUsableItemSlot(id);
    if ((slot & 0xFFFF) != SLOT_EMPTY) {
        p += slot >> 16;
        left = (*p >> ITEM_SHIFT) - 1;
        if (left <= 0) {
            *p = 0;
        } else {
            *p = id | (left & ITEM_COUNT_MASK) << ITEM_SHIFT;
        }
    }
}

/* Unlike BtlItemSlot this one has no separate answer for a matched slot that
   is empty: any slot with room reads as SLOT_SOME. */
unsigned int BtlUsableItemSlot(u_char id)
{
    u_short *p;
    int      i;

    p = g_btl_usable_items;
    i = 0;
    do {
        if ((*p & ITEM_ID) == id) {
            if (*p >> ITEM_SHIFT < ITEM_MAX) {
                return i << 16 | SLOT_SOME;
            }
            return i << 16;
        }
        i++;
        p++;
    } while (i < USABLE_ITEM_SLOTS);

    p = g_btl_usable_items;
    i = 0;
    do {
        if ((*p & ITEM_ID) == 0) {
            return i << 16 | SLOT_EMPTY;
        }
        i++;
        p++;
    } while (i < USABLE_ITEM_SLOTS);
}
