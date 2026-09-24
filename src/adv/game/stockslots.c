/* Persona 1 (JP) - ADV's Persona stock slot helpers.
 *
 *   ADV @ 0x800AF800, 0x800AF848, 0x800AF880, 0x800AF914
 *
 * The stock itself and its compaction are shared with DNG and S2D and live in
 * src/common/game/personastock.c. These three reach into the same fifteen
 * slots but are only ever linked into ADV, in a unit of their own well past
 * the shared code.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/persona.h>

#define STOCK_SLOTS 15
#define NO_SLOT     0xFF

extern u_char PersonaStockFindFree(void);

/* The slot holding a Persona, or 0xFF. */
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

/* Nothing checks that there was an empty slot: a full stock stores through
   0xFF and writes past the array. */
void PersonaStockAdd(u_char id)
{
    g_persona_stock[PersonaStockFindFree()] = id;
}

/* Puts a Persona back in the stock and closes the gaps. The slot it is
   written to is never worked out - the index is whatever the register held,
   which PersonaStockAdd's search would have left there - so the store only
   lands where it should if the caller has just searched. */
void PersonaStockReturn(u_char id)
{
    u_char *stock;
    u_char  slot;
    u_char  i;
    u_char  j;

    g_persona_stock[slot] = id;
    stock = g_persona_stock;
    for (i = 0; i < STOCK_SLOTS; i++) {
        if (stock[i] == STOCK_FREE) {
            for (j = i + 1; j < STOCK_SLOTS; j++) {
                if (stock[j] != STOCK_FREE) {
                    stock[i] = stock[j];
                    stock[j] = STOCK_FREE;
                    break;
                }
            }
        }
    }
}

/* The first empty slot, or 0xFF. */
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
