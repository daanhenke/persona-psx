/* Persona 1 (JP) - searching the character records.  ADV only.
 *   0x800AF9D4 CharFind2   0x800AFA34 CharFindFree   0x800AFA80 CharFind
 *
 * The middle of the three units this source was cut into; PartyAdd is in
 * chars.c and the Persona and list searches in charslots.c.
 *
 * A record identifies itself by the byte at +0x00, and a key of zero means the
 * slot is free. All three answer 0xFF when nothing matches.
 */
#include <decomp/types.h>
#include <persona/common/char.h>

/* The same search twice over, with the counter a short in one and a u_char in
   the other, so the bound test comes out signed here and unsigned in CharFind
   below. Both are in the image; neither is dead. */
u_char CharFind2(u_char key)
{
    short i;

    for (i = 0; i < CHAR_COUNT; i++) {
        if (g_chars[i].key == key) {
            return i;
        }
    }
    return 0xFF;
}

u_char CharFindFree(void)
{
    u_char i;

    for (i = 0; i < CHAR_COUNT; i++) {
        if (g_chars[i].key == 0) {
            return i;
        }
    }
    return 0xFF;
}

u_char CharFind(u_char key)
{
    u_char i;

    for (i = 0; i < CHAR_COUNT; i++) {
        if (g_chars[i].key == key) {
            return i;
        }
    }
    return 0xFF;
}
