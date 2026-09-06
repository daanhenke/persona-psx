/* Persona 1 (JP) - the stat line for a slot of the pending item list.
 *
 *   ADV @ 0x8006ABEC   DNG @ 0x8007A150   S2D @ 0x8006A178
 *
 * A slot counts as holding something only when both halves of its packed entry
 * are set. An empty one asks for record 0 instead, which is the blank
 * placeholder an empty equipment slot resolves to, so the row comes out clear
 * rather than being skipped.
 */
#include <types.h>

/* The staging list, reached by hardcoded address like the rest of the overlay
   work area. S2D's sits 0x20000 higher. */
#define g_items_pending ((u_short *)(0x800EAE4C + WORK_BIAS))

#define ITEM_ID    0x1FF
#define ITEM_SHIFT 9

/* Where the row is built and how wide it comes out. */
#define STAT_ROW_AT    0x38
#define STAT_ROW_WIDTH 0xE

extern u_char *TextItemStatRow(int id, int at, int width);

void TextSlotStatRow(short slot)
{
    u_short *entry;
    int      id;

    entry = &g_items_pending[slot];
    if ((*entry >> ITEM_SHIFT) != 0) {
        id = *entry & ITEM_ID;
        if (id != 0) {
            goto found;
        }
    }
    id = 0;
found:
    TextItemStatRow(id, STAT_ROW_AT, STAT_ROW_WIDTH);
}
