/* Persona 1 (JP) - a party member's sound bank.  BTLP only.
 *   0x8009239C BtlLoadMemberBank   0x800923F4 BtlReadMemberBank
 *   0x80092468 BtlOpenMemberBank
 *
 * The party's twin of packload.c and packbank.c. A member's bank is an entry
 * of the battle's pack picked by the member's key, and it arrives in the same
 * buffer and is opened into the same slot as a demon's voice bank - only the
 * fighter whose turn comes next has its bank in, so the two never stand
 * together. The header and the sequence are moved to the same homes too. What
 * differs is that a member's sequence holds seven sub-sequences, and the first
 * five are put back to full volume as the bank opens.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <libsnd.h>
#include <persona/btlp/number.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/sound.h>

#define MEMBER_BANK_SEPS   7
#define MEMBER_BANK_VOICES 5
#define MEMBER_BANK_VOL    0x7F

void BtlLoadMemberBank(int key)
{
    /* The bank's four-word header, which BtlOpenMemberBank re-reads out of
       the buffer. Nothing here touches it, but the frame keeps its slot, as
       BtlLoadPackBank's does. */
    u_long header[4];

    BtlReadSectors((u_long *)BTL_PACK_BUFFER,
                   g_btl_member_bank_sectors[key] + g_btl_pack_base,
                   g_btl_member_bank_sectors[key + 1]
                       - g_btl_member_bank_sectors[key]);
    BtlOpenMemberBank();
}

void BtlReadMemberBank(int wait, int key)
{
    int sector;
    int sectors;

    sector = g_btl_member_bank_sectors[key] + g_btl_pack_base;
    sectors = g_btl_member_bank_sectors[key + 1]
              - g_btl_member_bank_sectors[key];
    if (wait != 0) {
        BtlReadSectors((u_long *)BTL_PACK_BUFFER, sector, sectors);
        g_btl_pack_ready = 1;
    } else {
        BtlReadSectorsAsync((u_long *)BTL_PACK_BUFFER, sector, sectors);
    }
}

void BtlOpenMemberBank(void)
{
    BtlSoundBank bank;
    int          i;

    memcpy((u_char *)BTL_PACK_VH_HOME, g_btl_pack_vh, BTL_PACK_VH_SIZE);
    memcpy((u_char *)BTL_PACK_SEQ_HOME, g_btl_pack_seq, BTL_PACK_SEQ_SIZE);

    bank.vh = (u_char *)BTL_PACK_VH_HOME;
    bank.vb = g_btl_pack_vb;
    bank.seq = (u_long *)BTL_PACK_SEQ_HOME;
    bank.nsep = MEMBER_BANK_SEPS;
    BtlSoundOpen(&bank, BTL_PACK_SLOT, 0);

    for (i = 0; i < MEMBER_BANK_VOICES; i++) {
        SsSepSetVol(g_btl_seq[BTL_PACK_SLOT], i, MEMBER_BANK_VOL,
                    MEMBER_BANK_VOL);
    }
    BtlDrawNumberAlt(g_btl_bank_kind_cells, BTL_BANK_MEMBER, BTL_BANK_CELLS);
}
