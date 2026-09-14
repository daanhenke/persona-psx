/* Persona 1 (JP) - opening the bank a pack was read into.  BTLP only.
 *   0x800921B4 BtlOpenPackBank
 *
 * The pack arrives in one read, and two of its three pieces have to be moved
 * before the SPU can be told about them: the VAB header and the SEQ each have
 * a fixed home in the resident work area. The VAB body is left where it landed
 * because BtlSoundOpen hands that one straight to the SPU without reading it.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/number.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/sound.h>

/* How many sub-sequences a demon's SEQ holds. */
#define BTL_PACK_SEPS 10

void BtlOpenPackBank(void)
{
    BtlSoundBank bank;

    memcpy((u_char *)BTL_PACK_VH_HOME, g_btl_pack_vh, BTL_PACK_VH_SIZE);
    memcpy((u_char *)BTL_PACK_SEQ_HOME, g_btl_pack_seq, BTL_PACK_SEQ_SIZE);

    bank.vh = (u_char *)BTL_PACK_VH_HOME;
    bank.vb = g_btl_pack_vb;
    bank.seq = (u_long *)BTL_PACK_SEQ_HOME;
    bank.nsep = BTL_PACK_SEPS;
    BtlSoundOpen(&bank, BTL_PACK_SLOT, 0);

    BtlDrawNumberAlt(g_btl_bank_kind_cells, BTL_BANK_DEMON, BTL_BANK_CELLS);
}
