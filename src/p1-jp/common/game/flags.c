/* Persona 1 (JP) - flag bank accessors.
 *
 * Five contiguous routines in one translation unit, compiled into more than
 * one overlay rather than called across the boundary:
 *   ADV @ 0x80081FF4 / 0x8008202C / 0x8008206C / 0x800820A4 / 0x800820E4
 *   DNG @ 0x8008BF50 (get) and S2D @ 0x8007C3B4 (get) are the same code.
 * The banks live in the shared save-game work area, so their addresses are the
 * same in every overlay and one source covers all of them.
 *
 * Three 0x40-byte banks sit next to each other - 512 flags each:
 *   0x801F29C8  g_event_flags   the story flags EventFlagTest also reads
 *   0x801F2A08  g_flags_bank1
 *   0x801F2A48  g_flags_bank2
 * What the second and third hold is not established yet; they are named for
 * their shape, which is all that is observable so far. Bank 1 has a getter and
 * a setter but no clear, bank 2 only a getter, and that getter lives in a
 * different translation unit (0x80098A08).
 *
 * These return the masked bit, not 0 or 1 - unlike EventFlagTest in
 * eventflag.c, which shifts the bit down. Callers only test for zero.
 *
 * The id is signed and the index is id / 8, so a negative id reads backwards
 * off the front of the bank rather than wrapping.
 */
#include <types.h>

#define g_event_flags ((u_char *)0x801F29C8)
#define g_flags_bank1 ((u_char *)0x801F2A08)

int FlagBank1Get(short id)
{
    u_char *p;
    int     v;

    p = g_flags_bank1;
    v = p[id / 8];
    v = v & (1 << (id & 7));
    return v;
}

void FlagBank1Set(short id)
{
    u_char *p;
    int     v;

    p = g_flags_bank1;
    v = p[id / 8];
    v = v | (1 << (id & 7));
    p[id / 8] = v;
}

int EventFlagGet(short id)
{
    u_char *p;
    int     v;

    p = g_event_flags;
    v = p[id / 8];
    v = v & (1 << (id & 7));
    return v;
}

void EventFlagSet(short id)
{
    u_char *p;
    int     v;

    p = g_event_flags;
    v = p[id / 8];
    v = v | (1 << (id & 7));
    p[id / 8] = v;
}

void EventFlagClear(short id)
{
    u_char *p;
    int     v;

    p = g_event_flags;
    v = p[id / 8];
    v = v & ~(1 << (id & 7));
    p[id / 8] = v;
}
