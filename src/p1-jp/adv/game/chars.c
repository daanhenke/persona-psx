/* Persona 1 (JP) - lookups over the character records.  ADV only.
 *   0x800ADBC0 PartyAdd          0x800AFA34 CharFindFree
 *   0x800AFA80 CharFind          0x800B0440 CharEntryFind
 *   0x800AF9D4 CharFind2         0x800B04E0 CharEntryFindFree
 *
 * Five 0x60-byte records in the save-game work area, reached through g_party,
 * which holds a record index per party slot.
 */
#include <types.h>
#include <persona/common/char.h>

#define g_party ((u_char *)0x801F256C)

extern u_char PartyFindSlot(u_char chr);

/* Puts a character in the first empty party slot. Nothing checks the party has
   room: a full party gets 0xFF back and writes one past the end. */
void PartyAdd(u_char chr)
{
    g_party[PartyFindSlot(0xFF)] = chr;
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

/* The same search compiled in another translation unit, where the counter is a
   short, so the bound test comes out signed here and unsigned in CharFind. */
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
