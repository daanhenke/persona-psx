/* Persona 1 (JP) - per-state preloads.  SLPS_005.00
 *
 * main calls one of these before handing control to an overlay, so the CD read
 * is already in flight by the time the overlay starts. Which one it calls is
 * confirmed twice over: by the state main then sets, and by the file queued.
 */
#include <libcd.h>
#include <persona/common/eventflag.h>
#include <persona/main/cd.h>

/* Scratch CdlFILE for the ADV scene. Its size slot holds a *sector count*,
   which is why PreloadAdv shifts it left by 11 to get the byte size. */
extern CdlFILE g_adv_scene_file;
extern u_char  g_adv_scene_arg[];

extern short g_map_id[];
extern int   g_state_next;
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
extern void  FormatHexDigits(int value, char *end, short digits);

extern u_short g_save_map_id[];
extern u_short g_save_unk4[];   /* u16 here; main reads the same byte as u8 */
extern u_char  g_save_pos_x[];
extern u_char  g_save_pos_y[];
extern u_char  g_save_unk5_idx[];
extern u_char  g_map_pos_x[];
extern u_char  g_map_pos_y[];
extern u_char  g_map_unk4[];
extern u_char  g_map_room[];
extern u_char  D_8004D9E4[];
extern u_char  D_8004DA14[];
extern u_char  g_dng_third_gate[];
extern u_char  g_dng_third_flags[];

/* The three DNG filename templates, wrapped in structs so PreloadDng can take
   a fresh copy of one with a plain assignment: "\Dxx\Dyy.BIN;1" is 15 bytes
   with its terminator, the S and M variants 16. */
typedef struct { char c[15]; } DngName15;
typedef struct { char c[16]; } DngName16;
extern const DngName15 str_dng_tmpl;
extern const DngName16 str_dngs_tmpl;
extern const DngName16 str_dngm_tmpl;

#define DNG_DEST   ((void *)0x80130000)
#define DNG_M_DEST ((void *)0x801CA000)
#define DNG_S_DEST ((void *)0x801CD000)

extern void AdvResolveSceneLoc(short kind, short index, void *unused);

#define ADV_DEST  ((void *)0x80180000)
#define ADV_SCENE_DEST ((void *)0x801B8000)
#define NAME_DEST ((void *)0x80140000)
#define S2D_MDL_DEST ((void *)0x80140000)
#define S2D_MAP_DEST ((void *)0x80190000)

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

/* State 5 (NAME): one asynchronous read, nothing else. */
void PreloadName(void)
{
    LoadFileToAddrAsync(str_namedt_bin, NAME_DEST);
}

/* State 0 (DNG): three filenames of the form \Dxx\Dyy[SM].BIN;1, where yy is
   the floor in hex and xx the floor group (floor >> 3). Each template is
   copied into a local first, so every call starts from a clean one. The third
   file is skipped when entering from ADV with the gate flags clear. */
void PreloadDng(void)
{
    DngName15 name;
    DngName16 names;
    DngName16 namem;
    int       count;

    name = str_dng_tmpl;
    names = str_dngs_tmpl;
    namem = str_dngm_tmpl;

    /* Coming back from BTLP or a cutscene keeps the floor you were on. */
    if (g_state_next != 1 && g_state_next != 6) {
        g_save_map_id[0] = g_map_id[0];
        g_save_unk4[0] = g_map_unk4[0];
        g_save_pos_x[0] = g_map_pos_x[0];
        g_save_pos_y[0] = g_map_pos_y[0];
        g_save_unk5_idx[0] = D_8004DA14[g_map_room[0] * 4];
    }
    if (D_8004D9E4[g_save_map_id[0]] == 0 || D_8004D9E4[g_save_map_id[0]] > 0x24) {
        g_save_map_id[0] = 0;
    }
    if (D_8004D9E4[g_save_map_id[0]] <= g_save_unk4[0]) {
        g_save_unk4[0] = 0;
    }

    FormatHexDigits((short)g_save_map_id[0], &name.c[7], 2);
    names.c[7] = name.c[7];
    names.c[6] = name.c[6];
    namem.c[7] = name.c[7];
    namem.c[6] = name.c[6];

    FormatHexDigits(g_save_map_id[0] >> 3, &name.c[3], 2);
    g_cd_queue[0].name = name.c;
    g_cd_queue[0].dest = DNG_DEST;
    g_cd_queue[1].name = namem.c;
    g_cd_queue[0].mode = 0;
    g_cd_queue[1].dest = DNG_M_DEST;
    g_cd_queue[2].name = names.c;
    g_cd_queue[1].mode = 0;
    g_cd_queue[2].dest = DNG_S_DEST;
    g_cd_queue[2].mode = 0;

    names.c[3] = name.c[3];
    names.c[2] = name.c[2];
    namem.c[3] = name.c[3];
    namem.c[2] = name.c[2];

    count = 3;
    if (g_state_next == 3 && g_dng_third_gate[0] == 0 && (g_dng_third_flags[0] & 1) == 0) {
        count = 2;
    }
    CdQueueSubmit(count);
}

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

/* Writes `digits` hex digits of `value` backwards from `end`, so the caller
   passes a pointer to the *last* digit position. Immediately follows
   PreloadDng in the binary, so it belongs to this translation unit. */
void FormatHexDigits(int value, char *end, short digits)
{
    short i;
    int   v;
    short rem;
    char  c;

    i = 0;
    if (digits > 0) {
        do {
            v = (short)value;
            value = v / 16;
            rem = v - value * 16;
            c = (char)rem;
            c = c + 0x30;
            if (rem > 9) {
                c = (char)rem + 0x37;
            }
            *end = c;
            i++;
            end--;
        } while (i < digits);
    }
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
