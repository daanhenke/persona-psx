/* Persona 1 (JP) - per-state preloads.  SLPS_005.00
 *   0x800156D4 PreloadS2d   0x80015890 PreloadAdv   0x80015EB8 PreloadName
 *
 * main calls one of these before handing control to an overlay, so the CD read
 * is already in flight by the time the overlay starts. Which one it calls is
 * confirmed twice over: by the state main then sets, and by the file queued.
 *
 * The dungeon preload is a unit of its own earlier in the image, in
 * preloaddng.c. Three routines between PreloadAdv and PreloadName here do not
 * come out of the C yet and are taken from asm in place, which is what keeps
 * this one unit rather than three.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <persona/common/eventflag.h>
#include <persona/main/cd.h>

/* Scratch CdlFILE for the ADV scene. Its size slot holds a *sector count*,
   which is why PreloadAdv shifts it left by 11 to get the byte size. */
extern CdlFILE g_adv_scene_file;
extern u_char  g_adv_scene_arg[];

extern short  g_map_id[];
extern int    g_state_next;
extern u_char g_btl_map_id[];
extern u_char g_btl_pos_y[];

/* Patched in place before each strcpy, so these are data, not rodata. */
extern char str_2dmdl_tmpl[];
extern char str_exmap_tmpl[];
extern char g_cd_name_buf[2][0x14];

extern const char str_advchr_bin[];
extern const char str_adv_bst_bin[];
extern const char str_adv_dvl_bin[];
extern const char str_adv_per_bin[];
extern const char str_adv_e0_bin[];
extern const char str_adv_e1_bin[];
extern const char str_adv_e2_bin[];
extern const char str_adv_e3_bin[];
extern u_short g_adv_e0_offsets[];
extern u_short g_adv_e1_offsets[];
extern u_short g_adv_e2_offsets[];
extern u_short g_adv_e3_offsets[];
extern const char str_namedt_bin[];

extern char *strcpy(char *dst, const char *src);

extern void AdvResolveSceneLoc(short kind, short index, void *unused);

#define ADV_DEST  ((void *)0x80180000)
#define ADV_SCENE_DEST ((void *)0x801B8000)
#define NAME_DEST ((void *)0x80140000)
#define S2D_MDL_DEST ((void *)0x80140000)
#define S2D_MAP_DEST ((void *)0x80190000)

/* State 2 (S2D): advances the area past any story gates that have opened, then
   builds both 2D filenames by patching a digit into a template in place and
   copying the result into a scratch buffer the queue entry points at.
 *
 * The area id at 0x801F5350 and the y position at 0x801F5353 are reached by
 * hardcoded address here rather than through their named symbols. */
void PreloadS2d(void)
{
    u_short *map;
    u_char  *posy;
    u_short  flag;
    u_char   half;

    map = (u_short *)0x801F5350;
    posy = (u_char *)0x801F5353;

    /* Area 0 advances to 1 once event flag 0x14 is set and to 2 at 0x27; area
       3 advances to 5 at 0x67 and to 6 at 0x6E. Every other area is fixed. */
    switch (*map) {
    case 0:
        flag = 0x14;
        if (EventFlagTest(&flag) == 1) {
            *map = 1;
        }
        flag = 0x27;
        if (EventFlagTest(&flag) == 1) {
            *map = 2;
        }
        break;
    case 3:
        flag = 0x67;
        if (EventFlagTest(&flag) == 1) {
            *map = 5;
        }
        flag = 0x6E;
        if (EventFlagTest(&flag) == 1) {
            *map = 6;
        }
        break;
    }

    if (g_state_next == 1) {
        *map = g_btl_map_id[0];
        *posy = g_btl_pos_y[0];
    }

    str_2dmdl_tmpl[10] = *(u_char *)map + '0';
    strcpy(g_cd_name_buf[0], str_2dmdl_tmpl);
    g_cd_queue[0].name = g_cd_name_buf[0];
    g_cd_queue[0].dest = S2D_MDL_DEST;
    g_cd_queue[0].mode = 0;

    half = 'A';
    str_exmap_tmpl[10] = *(u_char *)map + '0';
    if (*posy > 0x90) {
        half = 'B';
    }
    str_exmap_tmpl[11] = half;
    strcpy(g_cd_name_buf[1], str_exmap_tmpl);
    g_cd_queue[1].name = g_cd_name_buf[1];
    g_cd_queue[1].dest = S2D_MAP_DEST;
    g_cd_queue[1].mode = 0;

    CdQueueSubmit(2);
}

/* State 3 (ADV): the resolved scene, then the character bank. */
void PreloadAdv(void)
{
    CdlLOC *loc;

    AdvResolveSceneLoc(0, g_map_id[0], g_adv_scene_arg);

    loc = (CdlLOC *)&g_cd_queue[0].loc;
    CdIntToPos(CdPosToInt(&g_adv_scene_file.pos), loc);
    g_cd_queue[0].size = g_adv_scene_file.size << 11;
    g_cd_queue[0].mode = 0;

    CdSearchFileLoc((CdlFILE *)(loc + 9), str_advchr_bin);
    g_cd_queue[1].dest = ADV_DEST;
    g_cd_queue[1].mode = 0;

    CdQueueSubmitResolved(2);
}

/* Resolves an ADV scene to a CD position in g_adv_scene_file.
 *
 * kind 0 indexes one of four scene packs by the high byte of `index`: the pack
 * has a u16 start-sector table, and the difference between entry i and i+1 is
 * the scene's length. Kinds 3, 4 and 5 are flat files with a fixed stride,
 * where entry i starts one sector in plus i * stride: 9 sectors an entry for
 * ADV_DVL.BIN, 8 for ADV_PER.BIN, 5 for ADV_BST.BIN.
 *
 * The size slot is left holding a *sector count*, not bytes; PreloadAdv is
 * what shifts it left by 11. */
#ifdef NON_MATCHING
void AdvResolveSceneLoc(short kind, short index, void *unused)
{
    int      base;
    int      off;
    int      pack;
    int      slot;
    u_short *tbl;

    switch (kind) {
    case 0:
        pack = index / 256;
        switch (pack) {
        case 1:
            CdSearchFileLoc(&g_adv_scene_file, str_adv_e1_bin);
            base = CdPosToInt(&g_adv_scene_file.pos);
            tbl = g_adv_e1_offsets;
            break;
        case 0:
            CdSearchFileLoc(&g_adv_scene_file, str_adv_e0_bin);
            base = CdPosToInt(&g_adv_scene_file.pos);
            tbl = g_adv_e0_offsets;
            break;
        case 2:
            CdSearchFileLoc(&g_adv_scene_file, str_adv_e2_bin);
            base = CdPosToInt(&g_adv_scene_file.pos);
            tbl = g_adv_e2_offsets;
            break;
        case 3:
            CdSearchFileLoc(&g_adv_scene_file, str_adv_e3_bin);
            base = CdPosToInt(&g_adv_scene_file.pos);
            tbl = g_adv_e3_offsets;
            break;
        default:
            goto out;
        }
        slot = index & 0xFF;
        CdIntToPos(base + tbl[slot], &g_adv_scene_file.pos);
        g_adv_scene_file.size = tbl[slot + 1] - tbl[slot];
out:
        g_cd_queue[0].dest = ADV_SCENE_DEST;
        return;
    case 4:
        CdSearchFileLoc(&g_adv_scene_file, str_adv_dvl_bin);
        g_adv_scene_file.size = 9;
        base = CdPosToInt(&g_adv_scene_file.pos) + 1;
        off = index * 9;
        break;
    case 5:
        CdSearchFileLoc(&g_adv_scene_file, str_adv_per_bin);
        g_adv_scene_file.size = 8;
        base = CdPosToInt(&g_adv_scene_file.pos) + 1;
        off = index * 8;
        break;
    case 3:
        CdSearchFileLoc(&g_adv_scene_file, str_adv_bst_bin);
        g_adv_scene_file.size = 5;
        base = CdPosToInt(&g_adv_scene_file.pos) + 1;
        off = index * 5;
        break;
    default:
        return;
    }

    CdIntToPos(base + off, &g_adv_scene_file.pos);
}
#else
INCLUDE_ASM("main/nonmatchings/preload", AdvResolveSceneLoc);
#endif

/* Two more this file carries but does not work out: a helper of the resolver
   and the routine that builds a Persona record from a pack. */
INCLUDE_ASM("main/nonmatchings/preload", PreloadBtlField);

INCLUDE_ASM("main/nonmatchings/preload", PersonaCreate);

/* State 5 (NAME): one asynchronous read, nothing else. */
void PreloadName(void)
{
    LoadFileToAddrAsync(str_namedt_bin, NAME_DEST);
}

