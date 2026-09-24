/* Persona 1 (JP) - finding one resource inside ADV's archives.  ADV only.
 *   0x80086464 AdvSelectFile
 *
 * ADV's data is packed into a handful of archives on the disc. Given a kind
 * and an id, this looks the archive up and narrows g_adv_scene_file down to
 * the one resource: its position moves to the resource's first sector and
 * its size becomes the resource's length in sectors. Most archives carry a
 * table of sector offsets, one per id and one past the end; the scene
 * scripts and the shadow sets are fixed-size, one sector in.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

#define FILE_MES   1
#define FILE_EBG   2
#define FILE_BST   3
#define FILE_KAGE  6
#define FILE_BGM   7
#define FILE_BVB   8
#define FILE_SE    9
#define FILE_SVB   10

/* The archives' paths, "\ADV\MES.BIN;1" and so on. They sit in rodata just
   ahead of this unit's own jump table, in a unit of their own. */
extern const char str_adv_mes[];
extern const char str_adv_ebg[];
extern const char str_adv_bst[];
extern const char str_adv_kage[];
extern const char str_adv_bgm[];
extern const char str_adv_bvb[];
extern const char str_adv_se[];
extern const char str_adv_svb[];

/* Sectors per scene script and per shadow set. */
#define BST_SECTORS  5
#define KAGE_SECTORS 0x4C

extern CdlFILE g_adv_scene_file;

extern u_short g_mes_offsets[];
extern u_short g_ebg_offsets[];
extern u_short g_bgm_offsets[];
extern u_short g_bvb_offsets[];
extern u_short g_se_offsets[];
extern u_short g_svb_offsets[];

/* Narrows the file to entry `id` of an archive with a sector table. Every
   kind spells this out in full; gcc merges the copies. */
#define SELECT_TABLE(name, table)                                             \
    CdSearchFileLoc(&g_adv_scene_file, name);                                 \
    CdIntToPos(CdPosToInt(&g_adv_scene_file.pos) + table[id],                \
               &g_adv_scene_file.pos);                                        \
    g_adv_scene_file.size = table[id + 1] - table[id]

/* The same for an archive of fixed-size entries, one sector in. */
#define SELECT_FIXED(name, sectors)                                           \
    CdSearchFileLoc(&g_adv_scene_file, name);                                 \
    g_adv_scene_file.size = sectors;                                          \
    CdIntToPos(CdPosToInt(&g_adv_scene_file.pos) + (1 + id * sectors),       \
               &g_adv_scene_file.pos)

void AdvSelectFile(short kind, short id)
{
    switch (kind) {
    case FILE_MES:
        SELECT_TABLE(str_adv_mes, g_mes_offsets);
        break;
    case FILE_EBG:
        SELECT_TABLE(str_adv_ebg, g_ebg_offsets);
        break;
    case FILE_BST:
        SELECT_FIXED(str_adv_bst, BST_SECTORS);
        break;
    case FILE_KAGE:
        SELECT_FIXED(str_adv_kage, KAGE_SECTORS);
        break;
    case FILE_BGM:
        SELECT_TABLE(str_adv_bgm, g_bgm_offsets);
        break;
    case FILE_BVB:
        SELECT_TABLE(str_adv_bvb, g_bvb_offsets);
        break;
    case FILE_SE:
        SELECT_TABLE(str_adv_se, g_se_offsets);
        break;
    case FILE_SVB:
        SELECT_TABLE(str_adv_svb, g_svb_offsets);
        break;
    }
}
