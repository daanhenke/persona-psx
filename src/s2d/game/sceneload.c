/* Persona 1 (JP) - S2D's disc loads: the map's sound, its scene data and
 * models, and the files it prefetches for whatever runs next.
 *
 * Every file here is named per map: the loader writes the map number's digit
 * into a fixed name ("\2D\2DSEQ00.BIN;1" becomes 2DSEQ03 on map 3) before
 * handing it to the main executable's LoadFileToAddr, so each name is a
 * writable array rather than a literal.
 *
 * The three S2dLeaveTo* stubs start the next overlay's read and say which
 * one it is; main loads it once S2D returns.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libcd.h>
#include <strings.h>
#include <persona/main/cd.h>
#include <persona/main/state.h>

extern short   g_bgm_ready;
extern u_char  g_map_pos_y;

/* The map's sequences and VAB headers are read here; the VAB body is staged
   at VAB_BODY_STAGE before it goes to sound RAM. */
#define SEQ_PACK       ((u_long *)0x801D8000)
#define VAB_BODY_STAGE ((u_char *)0x800D0000)
#define g_vab_id       ((short *)0x801F535C)

#define SCENE_AT   ((void *)0x80122000)
#define LTS_AT     ((void *)0x800D0000)
#define ADVCMD_AT  ((void *)0x800E0000)
#define MODELS_AT  0x80140000
#define EX_MAP_AT  0x80190000
#define MAP_ID_AT  ((short *)0x801F5350)

#define NAME_DIGIT 10

extern char str_2d_ex_map[];   /* "\2D\EX_MAP00.BIN;1" */
extern char str_2d_lts[];      /* "\2D\2DLTS00.BIN;1" */
extern char str_2d_scd[];      /* "\2D\2DSCD00.BIN;1" */
extern char str_adv_cmd[];     /* "\ADV\ADVCMD.BIN;1" */
extern char str_2d_seq[];      /* "\2D\2DSEQ00.BIN;1" */

extern CdlFILE g_lts_file;
extern CdlFILE g_advcmd_file;

/* Per map: the model id lists for the map itself and its two halves. */
extern u_char *g_map_model_ids[][3];
extern u_long  g_model_objs[256];
extern short   g_map_side;

extern short SsVabOpenHead(u_char *addr, short vabid);
extern short SsVabTransBody(u_char *addr, short vabid);
extern short SsVabTransCompleted(short immediateFlag);
extern void  VramLoad(int x, int y, int w, int h, u_long *p);

void S2dLoadMapModels(void);

/* The map's music: its sequence pack, unless ADV has just left the same
   music playing, then the VAB body. */
void S2dLoadMapSound(void)
{
    char  name[20];
    u_int map;

    map = (u_short)g_map_id;
    if (g_state_prev != GAME_STATE_ADV || g_bgm_ready == 1) {
        str_2d_seq[NAME_DIGIT] = map + '0';
        LoadFileToAddr(str_2d_seq, SEQ_PACK);
    }
    strcpy(name, "\\2D\\MAP00_WA.VB;1");
    name[8] = map + '0';
    do {
        g_vab_id[0] = SsVabOpenHead((u_char *)SEQ_PACK + *SEQ_PACK, -1);
    } while (g_vab_id[0] == -1);
    LoadFileToAddr(name, VAB_BODY_STAGE);
    SsVabTransBody(VAB_BODY_STAGE, g_vab_id[0]);
    SsVabTransCompleted(1);
}

void S2dLoadScene(void)
{
    str_2d_scd[NAME_DIGIT] = g_map_id + '0';
    LoadFileToAddr(str_2d_scd, SCENE_AT);
    S2dLoadMapModels();
}

void S2dLoadLtsAsync(void)
{
    str_2d_lts[NAME_DIGIT] = g_map_id + '0';
    LoadFileToAddrAsync(str_2d_lts, LTS_AT);
}

/* Parks the head on the LTS file so the read that follows starts at once. */
void S2dSeekLts(void)
{
    str_2d_lts[NAME_DIGIT] = g_map_id + '0';
    CdSearchFileLoc(&g_lts_file, str_2d_lts);
    CdControl(CdlSeekL, (u_char *)&g_lts_file, 0);
}

void S2dLoadLtsAsync2(void)
{
    str_2d_lts[NAME_DIGIT] = g_map_id + '0';
    LoadFileToAddrAsync(str_2d_lts, LTS_AT);
}

void S2dLeaveToDng(void)
{
    PreloadDng();
    g_state_next = GAME_STATE_DNG;
}

void S2dLeaveToAdv(void)
{
    PreloadAdv();
    g_state_next = GAME_STATE_ADV;
}

void S2dLeaveToBattle(void)
{
    PreloadBtlField();
    g_state_next = GAME_STATE_BTL;
}

/* Maps a TMD in place and points each listed object slot at its object,
   tagging the pointer's top two bits with `side`. `tmd` walks the header
   and then holds the object count: the image keeps both in one register. */
void S2dMapModels(int tmd, u_char *ids, int side)
{
    u_long *obj;
    int     i;

    tmd += 4;
    GsMapModelingData((u_long *)tmd);
    tmd += 4;
    obj = (u_long *)tmd + 1;
    tmd = *(int *)tmd;
    for (i = 0; i < tmd; i++) {
        g_model_objs[*ids] = (u_long)obj & 0x3FFFFFFF;
        g_model_objs[*ids] |= side << 30;
        *(u_long *)obj[4] &= 0xFF000000;
        *(u_long *)obj[4] |= obj[5];
        obj += 7;
        ids++;
    }
}

void S2dLoadMapModels(void)
{
    int     i;
    int     side;

    {
        u_long v = -1;
        for (i = 255; i >= 0; i--) {
            g_model_objs[i] = v;
        }
    }
    S2dMapModels(MODELS_AT, g_map_model_ids[*MAP_ID_AT][0], 0);
    S2dMapModels(EX_MAP_AT, g_map_model_ids[*MAP_ID_AT][2 - g_map_side],
                 2 - g_map_side);
    VramLoad(0x200, 0x100, 0x200, 0xE0, (u_long *)EX_MAP_AT);
    DrawSync(0);
    str_2d_ex_map[NAME_DIGIT] = *(u_char *)MAP_ID_AT + '0';
    str_2d_ex_map[NAME_DIGIT + 1] = g_map_pos_y >= 0x91 ? 'A' : 'B';
    LoadFileToAddr(str_2d_ex_map, (void *)EX_MAP_AT);
    side = g_map_side;
    S2dMapModels(EX_MAP_AT, g_map_model_ids[*MAP_ID_AT][side + 1], side + 1);
}

void S2dSeekAdvCmd(void)
{
    CdSearchFileLoc(&g_advcmd_file, str_adv_cmd);
    CdControl(CdlSeekL, (u_char *)&g_advcmd_file, 0);
}

void S2dLoadAdvCmdAsync(void)
{
    LoadFileToAddrAsync(str_adv_cmd, ADVCMD_AT);
}
