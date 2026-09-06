/* Persona 1 (JP) - pulling one pack bank off the disc.  BTLP only.
 *   0x800920E8 BtlLoadPackBank
 *
 * The pack holds a bank per demon, and its table of start offsets is
 * consecutive: an entry's end is where the next one begins, so the length of
 * a bank is the difference between neighbouring entries. The whole bank goes
 * to the one fixed buffer, which BtlOpenPackBank then unpacks into the three
 * pointers the sound code plays from - so only one demon's voice bank is
 * resident at a time, and callers reload it whenever the offer changes.
 */
#include <types.h>

extern u_short g_btl_pack_bank_sectors[];
extern int     g_btl_pack_bank_base;

extern void BtlReadSectors(u_long *dest, int sector, int sectors);
extern void BtlOpenPackBank(void);

void BtlLoadPackBank(int entry)
{
    /* The bank's four-word header. BtlOpenPackBank re-reads it out of the
       buffer, so nothing here touches it - but the slot is part of the frame
       and the function does not match without it. */
    u_long header[4];

    BtlReadSectors((u_long *)0x80152400,
                   g_btl_pack_bank_sectors[entry] + g_btl_pack_bank_base,
                   g_btl_pack_bank_sectors[entry + 1]
                       - g_btl_pack_bank_sectors[entry]);
    BtlOpenPackBank();
}
