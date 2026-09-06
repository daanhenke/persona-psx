/* Persona 1 (JP) - reading a sound bank off the disc and wiring it up.
 *   BTLP @ 0x80091DF4 BtlLoadSlotSound, 0x80091EC4 BtlLoadSound
 *
 * Both packs hold banks rather than raw data: an entry starts with three word
 * offsets - the VAB body, the VAB header and the SEQ - relative to wherever it
 * lands, and the loader turns those into the three pointers of a BtlSoundBank.
 * Two sub-sequences each. That is the same layout BtlBgmOpen unpacks by hand
 * for the battle music.
 *
 * The two differ only in which bank the result goes to. BtlLoadSound is indexed
 * directly; BtlLoadSlotSound asks g_btl_slot_owner which sound is sitting in
 * the slot it was given, and puts the bank in slot / 2 - 5, so slots 10 to 15
 * share three banks between them.
 */
#include <types.h>
#include <persona/btlp/sound.h>

/* Sub-sequences in every bank out of these two packs. */
#define BTL_PACK_SEPS 2

/* Slots 10..15 are the sound slots; the first of them owns bank 0. */
#define BTL_SLOT_BANK0 5

extern short        g_btl_slot_owner[];
extern u_short      g_btl_slot_sound_offsets[];
extern int          g_btl_slot_sound_base;
extern BtlSoundBank g_btl_slot_banks[];
extern u_short      g_btl_sound_offsets[];
extern int          g_btl_sound_base;
extern BtlSoundBank g_btl_banks[];

extern void BtlReadSectors(u_long *dest, int sector, int sectors);

/* Both of these index the bank array per field rather than taking a pointer to
   the record, and read the three offsets out of the loaded data before any of
   them is used. Tidying either costs the match. */

void BtlLoadSlotSound(u_long *dest, int slot)
{
    short  entry;
    int    bank;
    u_long vb;
    u_long vh;
    u_long seq;

    entry = g_btl_slot_owner[slot];
    BtlReadSectors(dest, g_btl_slot_sound_offsets[entry] + g_btl_slot_sound_base,
                   g_btl_slot_sound_offsets[entry + 1]
                       - g_btl_slot_sound_offsets[entry]);
    bank = slot / 2 - BTL_SLOT_BANK0;
    vb = dest[0];
    vh = dest[1];
    seq = dest[2];
    g_btl_slot_banks[bank].nsep = BTL_PACK_SEPS;
    g_btl_slot_banks[bank].vb = (u_char *)dest + vb;
    g_btl_slot_banks[bank].vh = (u_char *)dest + vh;
    g_btl_slot_banks[bank].seq = (u_long *)((u_char *)dest + seq);
}

void BtlLoadSound(u_long *dest, int entry)
{
    u_long vb;
    u_long vh;
    u_long seq;

    BtlReadSectors(dest, g_btl_sound_offsets[entry] + g_btl_sound_base,
                   g_btl_sound_offsets[entry + 1] - g_btl_sound_offsets[entry]);
    vb = dest[0];
    vh = dest[1];
    seq = dest[2];
    g_btl_banks[entry].nsep = BTL_PACK_SEPS;
    g_btl_banks[entry].vb = (u_char *)dest + vb;
    g_btl_banks[entry].vh = (u_char *)dest + vh;
    g_btl_banks[entry].seq = (u_long *)((u_char *)dest + seq);
}
