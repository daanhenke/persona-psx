/* Persona 1 (JP) - pointing the drive at one of the overlay's own files.
 *   BTLP @ 0x80067898 BtlSeekFile
 *
 * Each of the overlay's filenames is a 0x18-byte record; the located file is
 * kept in the scratch slot, which is marked unloaded so the next read fills
 * it.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

/* Each of the overlay's own filenames is a 0x18-byte record. */
#define BTL_FILE_LEN 0x18

extern u_char  g_btl_files[];
extern CdlFILE g_btl_scratch_file;
extern int     g_btl_scratch_loaded;

void BtlSeekFile(int index)
{
    CdSearchFileLoc(&g_btl_scratch_file, (char *)&g_btl_files[index * BTL_FILE_LEN]);
    CdControl(CdlSeekL, (u_char *)&g_btl_scratch_file, 0);
    g_btl_scratch_loaded = 0;
}
