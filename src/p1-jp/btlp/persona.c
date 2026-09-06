/* Persona 1 (JP) - which Persona an actor is fighting with.  BTLP only.
 *   0x800ACAF4 BtlActorPersona
 *
 * Each actor record opens with a Char, and a Char carries three list slots with
 * one of them marked active. This reads that slot; -1 says the actor has
 * nothing equipped. CharRecalcStats reads the same pair to work out the stats
 * the equipment and the Persona between them produce.
 */
#include <types.h>
#include <persona/btlp/actor.h>

int BtlActorPersona(int slot)
{
    u_char entry;

    entry = g_btl_actors[slot].c.entry;
    if (entry == CHAR_NO_ENTRY) {
        return -1;
    }
    return g_btl_actors[slot].c.list[entry];
}
