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
#include <persona/common/char.h>
#include <persona/common/persona.h>
#include <persona/btlp/actor.h>
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
extern const char str_bf_bin[];
extern const u_short g_bf_offsets[];

/* Which map each story encounter is fought on, by encounter id. */
#define ENC_STORY_COUNT 0x23
extern const u_char g_enc_story_map[ENC_STORY_COUNT];

/* Set once the party's actors have been cleared for the first battle. */
u_char g_btl_actors_cleared;

/* The encounter's map and the hero's surprise roll, as dng's field.h has them. */
#define g_enc_map      (*(u_char *)0x801F5354)
#define g_enc_surprise (*(u_char *)0x801F5355)

extern char *strcpy(char *dst, const char *src);

extern void AdvResolveSceneLoc(short kind, int index, void *unused);

#define ADV_DEST  ((void *)0x80180000)
#define ADV_SCENE_DEST ((void *)0x801B8000)
#define NAME_DEST ((void *)0x80140000)
#define BTL_FIELD_DEST ((u_long *)0x80140000)
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
/* 95.34%. The shape is the image's: case order 0,3,4,5, packs 0-3 in order,
   the flat files storing the size before CdPosToInt, and the next entry
   reached through a pointer taken as a value (&tbl[slot + 1]), which is what
   keeps CSE from folding it into &tbl[slot]. What is left is allocation: the
   image gives tbl and e the index's register (s1) and &tbl[slot] s3, which
   pins the andi above the table load; here tbl takes s3 and the andi moves
   into the jump's delay slot. The flat cases also build off in v0 rather
   than a0, so base is copied out of v0. */
#ifdef NON_MATCHING
void AdvResolveSceneLoc(short kind, int index, void *unused)
{
    CdlFILE *fp;
    CdlFILE *pk;
    int      base;
    int      pos;
    int      off;
    short    slot;
    u_short *tbl;
    u_short *e;

    switch (kind) {
    case 0:
        switch ((short)index / 256) {
        case 0:
            pk = &g_adv_scene_file;
            CdSearchFileLoc(pk, str_adv_e0_bin);
            pos = CdPosToInt(&pk->pos);
            slot = index & 0xFF;
            tbl = g_adv_e0_offsets;
            break;
        case 1:
            pk = &g_adv_scene_file;
            CdSearchFileLoc(pk, str_adv_e1_bin);
            pos = CdPosToInt(&pk->pos);
            slot = index & 0xFF;
            tbl = g_adv_e1_offsets;
            break;
        case 2:
            pk = &g_adv_scene_file;
            CdSearchFileLoc(pk, str_adv_e2_bin);
            pos = CdPosToInt(&pk->pos);
            slot = index & 0xFF;
            tbl = g_adv_e2_offsets;
            break;
        case 3:
            pk = &g_adv_scene_file;
            CdSearchFileLoc(pk, str_adv_e3_bin);
            pos = CdPosToInt(&pk->pos);
            slot = index & 0xFF;
            tbl = g_adv_e3_offsets;
            break;
        default:
            goto out;
        }
        CdIntToPos(pos + tbl[slot], &pk->pos);
        e = &tbl[slot + 1];
        g_adv_scene_file.size = *e - tbl[slot];
out:
        g_cd_queue[0].dest = ADV_SCENE_DEST;
        return;
    case 3:
        fp = &g_adv_scene_file;
        CdSearchFileLoc(fp, str_adv_bst_bin);
        g_adv_scene_file.size = 5;
        base = CdPosToInt(&fp->pos) + 1;
        off = (short)index * 5;
        break;
    case 4:
        fp = &g_adv_scene_file;
        CdSearchFileLoc(fp, str_adv_dvl_bin);
        g_adv_scene_file.size = 9;
        base = CdPosToInt(&fp->pos) + 1;
        off = (short)index * 9;
        break;
    case 5:
        fp = &g_adv_scene_file;
        CdSearchFileLoc(fp, str_adv_per_bin);
        g_adv_scene_file.size = 8;
        base = CdPosToInt(&fp->pos) + 1;
        off = (short)index * 8;
        break;
    default:
        return;
    }
    CdIntToPos(base + off, &fp->pos);
}
#else
INCLUDE_ASM("main/nonmatchings/preload", AdvResolveSceneLoc);
#endif

/* Before a battle: reads the encounter map's slice of BF.BIN, and clears the
   five party actors the first time a battle is entered.
 *
 * A story encounter (id under 0x23) picks its map from a table; a map id of
 * 0x80 and up is folded down by 0x44. Either way the surprise roll is reset.
 * g_bf_offsets holds each map's start sector in BF.BIN, entry i + 1 being
 * where it ends. */
void PreloadBtlField(void)
{
    CdlFILE file;
    CdlLOC  loc;
    u_char  map;

    map = g_enc_map;
    if ((u_short)g_map_id[0] < ENC_STORY_COUNT) {
        map = g_enc_story_map[(u_short)g_map_id[0]];
        g_enc_surprise = 0;
        g_enc_map = map;
    }
    if (map >= 0x80) {
        map -= 0x44;
        g_enc_map = map;
        g_enc_surprise = 0;
    }

    CdSearchFileLoc(&file, str_bf_bin);
    CdIntToPos(CdPosToInt(&file.pos) + g_bf_offsets[map], &loc);
    CdReadFileToAddrAsync((CdlFILE *)&loc, g_bf_offsets[map + 1] - g_bf_offsets[map],
                          BTL_FIELD_DEST);

    if (!g_btl_actors_cleared) {
        g_btl_actors_cleared = 1;
        bzero((u_char *)g_btl_actors, 5 * sizeof(BtlActor));
    }
}

/* Gives character `c` Persona `id`: the first free g_personas slot is filled
   out of the definition and becomes the only entry on the character's list.
   Returns the record, or the first one if all 31 are taken. */

Persona *PersonaCreate(Char *c, int id)
{
    Persona          *p;
    const PersonaDef *d;
    int               i;

    p = g_personas;
    d = &g_persona_defs[id];
    for (i = 0; i < PERSONA_COUNT; i++, p++) {
        if (p->key == 0) {
            p->unk00 = 0;
            p->unk04 = 0;
            p->unk08 = 0;
            p->bond = d->bond;
            p->unk10 = d->unk04;
            p->unk12 = d->unk06;
            p->key = id;
            memcpy(p->name, d->name, 10);
            p->sp_cost = d->sp_cost;
            p->level = d->level;
            p->kind = d->kind;
            p->stat[0] = d->stat[0];
            p->stat[1] = d->stat[1];
            p->stat[2] = d->stat[2];
            p->stat[3] = d->stat[3];
            p->stat[4] = d->stat[4];
            p->resist = d->resist;
            p->slots = 8;
            memcpy(p->spell, d->raw, PERSONA_SPELLS);
            memcpy((u_long *)p->raw, (u_long *)d->raw, PERSONA_SPELLS);
            p->unk3B = d->raw[6];
            p->pad3E = d->unk28;
            c->entry = 0;
            c->list[0] = i;
            c->list[1] = i + 1;
            c->list[2] = 0xFF;
            return p;
        }
    }
    return g_personas;
}

/* State 5 (NAME): one asynchronous read, nothing else. */
void PreloadName(void)
{
    LoadFileToAddrAsync(str_namedt_bin, NAME_DEST);
}

