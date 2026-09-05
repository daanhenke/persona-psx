/* Persona 1 (JP) - the Persona records.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG 0x800859E8   ADV 0x80076E6C   S2D 0x80075E24
 * The records live in the shared save-game work area, so one source covers all
 * three.
 */
#include <types.h>
#include <persona/common/persona.h>

#define SPELL_NONE 0
#define NO_SLOT    0xFF

/* Highest spell slot this Persona has learned, or 0xFF if it knows none.
 * Callers use it as the count to draw, so the scan runs downwards. */
u_char PersonaTopSpell(short id)
{
    Persona *p;
    int      i;

    p = &g_personas[id];
    for (i = PERSONA_SPELLS - 1; i >= 0; i--) {
        if (p->spell[i] != SPELL_NONE) {
            return i;
        }
    }
    return NO_SLOT;
}
