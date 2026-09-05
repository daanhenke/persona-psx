/* Persona 1 (JP) - the Persona stock.
 *
 * Fifteen slots in the save-game work area, each an index into the Persona
 * reference table, with 0 for an empty slot. The list screen at 0x800769E8
 * draws twelve of them, one row each: name, level, arcana and the stat bars.
 *
 * Only the compaction is compiled into all three overlays:
 *   DNG 0x800857A0   ADV 0x80076C24   S2D 0x80075BDC
 * The find/add helpers below are ADV's alone.
 */
#include <types.h>

#define g_persona_stock ((u_char *)0x801F297C)

#define STOCK_SLOTS 15
#define STOCK_ROWS  12
#define STOCK_FREE  0
#define NO_SLOT     0xFF

/* Slides the used slots down over the holes, then reports the last one still
   occupied. -1 says the stock is empty. Only the twelve rows the screen shows
   are tidied; the three slots past them are left where they are. */
short PersonaStockCompact(void)
{
    u_char *stock;
    int     i;
    int     j;

    stock = g_persona_stock;
    for (i = 0; i < STOCK_ROWS; i++) {
        if (stock[i] == STOCK_FREE) {
            for (j = i + 1; j < STOCK_ROWS; j++) {
                if (stock[j] != STOCK_FREE) {
                    stock[i] = stock[j];
                    stock[j] = STOCK_FREE;
                    break;
                }
            }
        }
    }
    for (i = STOCK_ROWS - 1; i >= 0; i--) {
        if (stock[i] != STOCK_FREE) {
            return i;
        }
    }
    return -1;
}

/* The slot holding a Persona, or 0xFF. ADV only. */
u_char PersonaStockFind(u_char id)
{
    u_char *stock;
    u_char  i;

    stock = g_persona_stock;
    for (i = 0; i < STOCK_SLOTS; i++) {
        if (stock[i] == id) {
            return i;
        }
    }
    return NO_SLOT;
}

/* The first empty slot, or 0xFF. ADV only. */
u_char PersonaStockFindFree(void)
{
    u_char *stock;
    u_char  i;

    stock = g_persona_stock;
    for (i = 0; i < STOCK_SLOTS; i++) {
        if (stock[i] == STOCK_FREE) {
            return i;
        }
    }
    return NO_SLOT;
}

/* Nothing checks that there was an empty slot: a full stock stores through
   0xFF and writes past the array. ADV only. */
void PersonaStockAdd(u_char id)
{
    g_persona_stock[PersonaStockFindFree()] = id;
}
