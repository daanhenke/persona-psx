/* Persona 1 (JP) - party slots and a u16 block copy.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x8008B86C / 0x80092AD4
 *   ADV @ 0x8007D2DC / 0x8008EA34
 *   S2D @ 0x8007BCDC / 0x80082FE8
 * Both live in the shared save-game work area, so one source covers all three.
 */
#include <types.h>
#include <persona/common/char.h>

/* Five slots, one byte each, 0xFF for empty. The byte indexes the 0x60-byte
   character records at 0x801F1BCC rather than being the character itself. */
#define g_party ((u_char *)0x801F256C)
#define PARTY_EMPTY 0xFF

/* Index of the last occupied slot.
 *
 * The loop has no lower bound - it walks down from 4 and stops at the first
 * slot that is not empty, so an entirely empty party would run off the front
 * of the array. That is the original's behaviour, not a transcription slip;
 * every caller reaches it with at least one member present. */
u_char PartyLastSlot(void)
{
    int slot;
    int i;

    slot = CHAR_COUNT - 1;
    for (;;) {
        i = (u_char)slot;
        slot--;
        if (g_party[i] != PARTY_EMPTY) {
            return i;
        }
    }
}

/* Count is in entries, and compared signed - a zero or negative count copies
   nothing. */
void CopyShorts(u_short *src, u_short *dst, u_short count)
{
    int i;

    for (i = 0; i < count; i++) {
        *dst = *src;
        src++;
        dst++;
    }
}

/* The slot holding the character with this key byte, or -1. Event scripts name
   a character by key and need the slot to reach the record. ADV only. */
short PartyFindByKey(u_char key)
{
    short  i;
    u_char chr;

    for (i = 0; i < CHAR_COUNT; i++) {
        chr = g_party[i];
        if (chr != PARTY_EMPTY && g_chars[chr].key == key) {
            return i;
        }
    }
    return -1;
}

/* The slot holding a given character, or 0xFF if they are not in the party.
   ADV only. */
u_char PartyFindSlot(u_char chr)
{
    u_char i;

    for (i = 0; i < 5; i++) {
        if (g_party[i] == chr) {
            return i;
        }
    }
    return 0xFF;
}
