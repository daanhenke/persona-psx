/* Persona 1 (JP) - searching the Persona records and a character's list.
 *   ADV only.
 *   0x800B03F8 PersonaFind      0x800B0440 CharEntryFind
 *   0x800B049C PersonaFindFree  0x800B04E0 CharEntryFindFree
 *
 * The last of the three units this source was cut into; PartyAdd is in chars.c
 * and the record searches in charfind.c.
 *
 * Thirty-one 0x40-byte Persona records sit directly after the character
 * records. They use the byte at +0x18 to identify themselves and answer -1
 * rather than 0xFF when nothing matches.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>

short PersonaFind(u_char key)
{
    u_char i;

    for (i = 0; i < PERSONA_COUNT; i++) {
        if (g_personas[i].key == key) {
            return i;
        }
    }
    return -1;
}

u_char CharEntryFind(u_char chr, u_char v)
{
    Char   *rec;
    u_char  i;

    rec = &g_chars[chr];
    for (i = 0; i < CHAR_LIST_N; i++) {
        if (rec->list[i] == v) {
            return i;
        }
    }
    return 0xFF;
}

short PersonaFindFree(void)
{
    u_char i;

    for (i = 0; i < PERSONA_COUNT; i++) {
        if (g_personas[i].key == 0) {
            return i;
        }
    }
    return -1;
}

u_char CharEntryFindFree(u_char chr)
{
    Char   *rec;
    u_char  i;

    rec = &g_chars[chr];
    for (i = 0; i < CHAR_LIST_N; i++) {
        if (rec->list[i] == 0xFF) {
            return i;
        }
    }
    return 0xFF;
}
