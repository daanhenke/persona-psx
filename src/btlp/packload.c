/* Persona 1 (JP) - pulling one pack bank off the disc.  BTLP only.
 *   0x800920E8 BtlLoadPackBank   0x80092140 BtlReadPackBank
 *
 * BtlLoadPackBank reads a bank and opens it in one breath; BtlReadPackBank
 * only reads, and takes the choice of waiting from its caller. The one that
 * does not wait leaves g_btl_pack_ready alone, so whoever asked for it can
 * tell the read is still running.
 */
#include <decomp/types.h>
#include <persona/btlp/pack.h>

void BtlLoadPackBank(int entry)
{
    /* The bank's four-word header. BtlOpenPackBank re-reads it out of the
       buffer, so nothing here touches it - but the slot is part of the frame
       and the function does not match without it. */
    u_long header[4];

    BtlReadSectors((u_long *)BTL_PACK_BUFFER,
                   g_btl_pack_bank_sectors[entry] + g_btl_pack_bank_base,
                   g_btl_pack_bank_sectors[entry + 1]
                       - g_btl_pack_bank_sectors[entry]);
    BtlOpenPackBank();
}

void BtlReadPackBank(int wait, int entry)
{
    int    sector;
    int    sectors;

    sector = g_btl_pack_bank_sectors[entry] + g_btl_pack_bank_base;
    sectors = g_btl_pack_bank_sectors[entry + 1]
              - g_btl_pack_bank_sectors[entry];
    if (wait != 0) {
        BtlReadSectors((u_long *)BTL_PACK_BUFFER, sector, sectors);
        g_btl_pack_ready = 1;
    } else {
        BtlReadSectorsAsync((u_long *)BTL_PACK_BUFFER, sector, sectors);
    }
}
