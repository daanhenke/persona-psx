/* Persona 1 (JP) - ADV's Persona stock slot helpers.
 *
 *   ADV @ 0x800AF800, 0x800AF848, 0x800AF914
 *
 * The stock itself and its compaction are shared with DNG and S2D and live in
 * src/common/game/personastock.c. These three reach into the same fifteen
 * slots but are only ever linked into ADV, in a unit of their own well past
 * the shared code. func_800AF880 sits between the add and the free-slot search
 * and so belongs to this unit; it has not been worked out yet.
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

INCLUDE_ASM("adv/nonmatchings/game/stockslots", func_800AF880);

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
