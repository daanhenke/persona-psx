/* Persona 1 (JP) - BTLP overlay @ 0x800678F8
 *
 * Fills the scratch buffer at 0x801C0000 from the disc. There are two ways in:
 *
 *   - by index, straight out of the overlay's own file table; or
 *   - the one big file whose location was searched for earlier, read
 *     asynchronously and only ever once - g_btl_scratch_loaded latches so a
 *     second call is free.
 *
 * Either way the loaded data starts with a header whose third word is its
 * length, and the address just past it is left in g_btl_scratch_end for
 * whoever unpacks it next.
 *
 * Both waits pump BtlDrawFrame rather than spinning, so the battle keeps
 * animating while the drive seeks.
 */
#include <types.h>
#include <libcd.h>

#define BTL_SCRATCH ((u_long *)0x801C0000)

extern CdlFILE g_btl_files[];
extern CdlFILE g_btl_scratch_file;
extern int     g_btl_scratch_loaded;
extern u_long *g_btl_scratch_end;
extern int     g_cd_busy;

extern void BtlDrawFrame(void);
extern void CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *addr);
extern void LoadFileToAddr(CdlFILE *file, u_long *addr);

void BtlLoadScratch(int index, int from_table)
{
    u_char res[8];

    if (from_table == 0) {
        if (g_btl_scratch_loaded == 0) {
            while (CdSync(1, res) != 2) {
                BtlDrawFrame();
            }
            CdReadFileToAddrAsync(&g_btl_scratch_file,
                                  (g_btl_scratch_file.size + 2047) >> 11,
                                  BTL_SCRATCH);
            while (g_cd_busy != -1) {
                BtlDrawFrame();
            }
            g_btl_scratch_loaded = 1;
        }
    } else {
        LoadFileToAddr(&g_btl_files[index], BTL_SCRATCH);
    }
    g_btl_scratch_end = (u_long *)((int)BTL_SCRATCH + (int)BTL_SCRATCH[2]);
}
