/* Persona 1 (JP) - which of a character's three entries is in use.
 *
 *   DNG 0x80085964   ADV 0x80076DE8   S2D 0x80075DA0
 *
 * The three bytes at +0x58 of a character record use 0xFF for empty; this
 * reports the highest occupied one. The byte at +0x5E overrides that: while it
 * is set the whole list reads as unusable.
 */
#include <types.h>
#include <persona/common/char.h>

#define g_party ((u_char *)0x801F256C)
#define ENTRY_EMPTY 0xFF

u_char CharTopEntry(short slot)
{
    Char  *rec;
    int    i;

    rec = &g_chars[g_party[slot]];
    if (rec->blocked) {
        return ENTRY_EMPTY;
    }
    for (i = CHAR_LIST_N - 1; i >= 0; i--) {
        if (rec->list[i] != ENTRY_EMPTY) {
            return i;
        }
    }
    return ENTRY_EMPTY;
}
