/* Persona 1 (JP) - how the battle overlay reads its data off the disc.
 *   BTLP @ 0x80091D2C, 0x80091DA8, 0x80092D88, 0x80066CCC, 0x80067898
 *
 * Most of what a battle needs lives in one packed file indexed by a table of
 * u16 start sectors: entry i runs from g_btl_pack_base + g_btl_pack_offsets[i]
 * to the next entry's start, so the table holds one more entry than there are
 * items and a length is always a subtraction. The ADV scene packs are laid out
 * the same way.
 *
 * The blocking read draws a frame every time round the wait, so an animation
 * already on screen keeps running while the next piece loads.
 */
#include <types.h>
#include <libcd.h>
#include <persona/main/cd.h>

/* Where a pack entry is read to. */
#define BTL_PACK_DEST ((u_long *)0x80152400)

/* Each of the overlay's own filenames is a 0x18-byte record. */
#define BTL_FILE_LEN 0x18

extern u_char   g_btl_files[];
extern CdlFILE  g_btl_scratch_file;
extern int      g_btl_scratch_loaded;
extern u_short  g_btl_pack_offsets[];
extern int      g_btl_pack_base;

extern void CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *dest);
extern void BtlDrawFrame(void);

void BtlReadSectors(u_long *dest, int sector, int sectors)
{
    CdlLOC loc;

    CdIntToPos(sector, &loc);
    CdReadFileToAddrAsync((CdlFILE *)&loc, sectors, dest);
    while (g_cd_busy != -1) {
        BtlDrawFrame();
    }
}

void BtlReadSectorsAsync(u_long *dest, int sector, int sectors)
{
    CdlLOC loc;

    CdIntToPos(sector, &loc);
    CdReadFileToAddrAsync((CdlFILE *)&loc, sectors, dest);
}

void BtlLoadPackEntry(int entry)
{
    BtlReadSectorsAsync(BTL_PACK_DEST,
                        g_btl_pack_offsets[entry] + g_btl_pack_base,
                        g_btl_pack_offsets[entry + 1]
                            - g_btl_pack_offsets[entry]);
}

/* Points the drive at an entry without reading it, so the seek is over by the
   time the read is asked for. */
void BtlSeekPackEntry(int entry)
{
    CdlLOC loc;

    CdIntToPos(g_btl_pack_offsets[entry] + g_btl_pack_base, &loc);
    CdControl(CdlSeekL, (u_char *)&loc, 0);
}

void BtlSeekFile(int index)
{
    CdSearchFileLoc(&g_btl_scratch_file, (char *)&g_btl_files[index * BTL_FILE_LEN]);
    CdControl(CdlSeekL, (u_char *)&g_btl_scratch_file, 0);
    g_btl_scratch_loaded = 0;
}
