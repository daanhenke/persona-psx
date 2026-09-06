/* Persona 1 (JP) - reading and setting a flag from the battle.  BTLP only.
 *   0x80067380 BtlFlagSet   0x800673B0 BtlFlagGet
 *
 * The third flag bank lives in the shared save-game work area, so what the
 * battle sets here is what the field reads afterwards. See common/game/flags.c
 * for the bank layout; this is only the setter, reached by hardcoded address
 * like the rest of that area.
 */
#include <types.h>

#define g_flags_bank3 ((u_char *)0x801F2A68)

/* The byte is reached through a pointer local rather than by indexing the
   literal, which is what keeps one base register for the load and the store. */
void BtlFlagSet(short n)
{
    u_char *p;

    p = g_flags_bank3 + (n >> 3);
    *p |= 1 << (n & 7);
}

int BtlFlagGet(short n)
{
    return g_flags_bank3[n >> 3] >> (n & 7) & 1;
}
