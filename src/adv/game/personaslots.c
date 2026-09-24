/* Persona 1 (JP) - the last filled Persona slot.  ADV only.
 *   0x800B0A90 PersonaSlotsLast
 *
 * Script command 59 jumps on how many of the sixteen Persona slots ahead of
 * the saved formations are in use, counted up to the last filled one.
 */
#include <decomp/types.h>

#define g_persona_slots ((u_char *)0x801F2574)

#define SLOT_EMPTY 0xFF

short PersonaSlotsLast(void)
{
    int i;

    for (i = 15; i >= 0; i--) {
        if (g_persona_slots[i] != SLOT_EMPTY) {
            return i;
        }
    }
    return -1;
}
