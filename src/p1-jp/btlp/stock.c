/* Persona 1 (JP) - adding to and taking from the Persona stock.  BTLP only.
 *   0x80070030 BtlStockAdd   0x8007006C BtlStockRemove
 *
 * The battle wins Personas and spends them, and it edits the same fifteen-slot
 * array the field keeps - but only the first twelve, which are the rows the
 * stock screen draws. Neither of these compacts the array afterwards: a
 * removal leaves its hole, and the field's own compaction closes it later.
 *
 * Adding to a full stock quietly does nothing, so callers ask BtlStockHasRoom
 * before offering the player one.
 */
#include <types.h>

#define g_persona_stock ((char *)0x801F297C)

#define STOCK_ROWS 12
#define STOCK_FREE 0

void BtlStockAdd(char id)
{
    char *stock;
    int   i;

    stock = g_persona_stock;
    i = 0;
    do {
        if (*stock == STOCK_FREE) {
            *stock = id;
            return;
        }
        i++;
        stock++;
    } while (i < STOCK_ROWS);
}

void BtlStockRemove(char id)
{
    char *stock;
    int   i;

    stock = g_persona_stock;
    i = 0;
    do {
        if (*stock == id) {
            *stock = STOCK_FREE;
            return;
        }
        i++;
        stock++;
    } while (i < STOCK_ROWS);
}
