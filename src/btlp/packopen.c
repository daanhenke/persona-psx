/* Persona 1 (JP) - reading the battle's pack entry and opening it as a bank.
 *   BTLP @ 0x80092D88 BtlLoadPackEntry, 0x80092DD8 BtlBgmOpen
 *
 * These two sit next to each other in the image although they were split
 * across the disc and sound sources by subject: an entry is read to
 * 0x80152400, whose first three words are the VAB body, the VAB header and the
 * SEQ, and opening it as slot 4 is the very next thing that happens.
 *
 * Entries are indexed by a table of u16 start sectors: entry i runs from
 * g_btl_pack_base + g_btl_pack_offsets[i] to the next entry's start, so the
 * table holds one more entry than there are items and a length is always a
 * subtraction.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/btlp/sound.h>

/* Where a pack entry is read to. */
#define BTL_PACK_DEST ((u_long *)0x80152400)

#define BTL_BGM_SEPS 4

extern u_short  g_btl_pack_offsets[];
extern int      g_btl_pack_base;

/* The head of whatever BtlLoadPackEntry last read. */
extern u_char *g_btl_pack_vb;
extern u_char *g_btl_pack_vh;
extern u_long *g_btl_pack_seq;

extern void BtlReadSectorsAsync(u_long *dest, int sector, int sectors);

void BtlLoadPackEntry(int entry)
{
    BtlReadSectorsAsync(BTL_PACK_DEST,
                        g_btl_pack_offsets[entry] + g_btl_pack_base,
                        g_btl_pack_offsets[entry + 1]
                            - g_btl_pack_offsets[entry]);
}

void BtlBgmOpen(void)
{
    BtlSoundBank bank;

    bank.nsep = BTL_BGM_SEPS;
    bank.vb = g_btl_pack_vb;
    bank.vh = g_btl_pack_vh;
    bank.seq = g_btl_pack_seq;
    BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
}
