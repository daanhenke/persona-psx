/* Persona 1 (JP) - opening the bank a pack was read into.  BTLP only.
 *   0x800921B4 BtlOpenPackBank
 *
 * The pack arrives in one read, and two of its three pieces have to be moved
 * before the SPU can be told about them: the VAB header and the SEQ each have
 * a fixed home in the resident work area. The VAB body is left where it landed
 * because BtlSoundOpen hands that one straight to the SPU without reading it.
 */
#include <types.h>
#include <libc.h>
#include <persona/btlp/sound.h>

/* Where the header and the sequence are moved to, and how much of each. */
#define BTL_PACK_VH_HOME  0x8016FCF0
#define BTL_PACK_SEQ_HOME 0x80171848
#define BTL_PACK_VH_SIZE  0x1B58
#define BTL_PACK_SEQ_SIZE 0xBB8

/* The slot this one takes, and how many sub-sequences its SEQ holds. */
#define BTL_PACK_SLOT 3
#define BTL_PACK_SEPS 10

extern u_char *g_btl_pack_vh;
extern u_char *g_btl_pack_vb;
extern u_long *g_btl_pack_seq;

extern int BtlDrawNumberAlt(u_char *dst, int value, int width);

/* A three-cell number field the layout table draws; what it counts is
   not settled, only that this is where it goes. */
extern u_char g_btl_number_cells[];

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

    BtlDrawNumberAlt(g_btl_number_cells, 1, BTL_PACK_SLOT);
}
