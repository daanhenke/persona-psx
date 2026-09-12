/* Persona 1 (JP) - the battle overlay's entry point.  BTLP only.
 *   0x8007F3E8 BtlBoxDismiss    0x8007F408 ovl_btlp_entry
 *
 * ovl_btlp_entry is what the resident EXE jumps to once the overlay is
 * loaded: it brings the fight up from the encounter the field handed over and
 * does not come back until the battle is finished with.
 *
 * The order is fixed and every step announces itself, which is why g_btl_debug
 * is tested between each one: on a development machine the announcement waits
 * for a key, so the run of strings below doubles as the sequence.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/box.h>
#include <persona/btlp/input.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/text.h>

/* The rest of the control scheme's masks. The four directions and the help key
   are the same under either scheme; these are the ones it moves. */
extern u_short g_btl_key_select;
extern u_short g_btl_key_square;
extern u_short g_btl_key_r1;
extern u_short g_btl_key_r2;
extern u_short g_btl_key_page;
extern u_short g_btl_key_end;

/* Which of the two schemes the player picked, in the options block. */
extern u_char g_pad_config;

/* Where the fight came from. g_map_id is the field the encounter was rolled
   on; D_8004E267 is the switch that says to take it rather than whatever
   g_btl_encounter already held. */
extern short  g_map_id;
extern u_char g_map_unk4;
extern u_char g_map_room;
extern u_char D_8004E267;
extern int    D_800EE618;
extern u_char D_800CCA2D;
extern int    g_state_prev;

/* The two scratch buffers past the overlay's own image, which the sound banks
   are read into. */
#define BTL_WORK_A ((u_char *)0x800F9148)
#define BTL_WORK_B ((u_char *)0x800FB768)

/* Where the background image goes: the field's buffer if the fight came
   straight off a map, the dungeon's otherwise. */
#define BTL_BG_FIELD ((u_char *)0x801B2000)
#define BTL_BG_DNG   ((u_char *)0x80130000)

/* What the loader left behind at 0x80140000: one address per run it read. Each
   is reached on its own, so they are separate names rather than one table.
   Once they have all been taken the run's own memory is reused, first as the
   battle's one-shade CLUT and then as the TIM scratch. */
#define g_load_stage (*(u_char **)0x80140000)
extern u_char *D_80140004;
extern u_char *D_80140010;
extern u_char *D_80140014;
extern u_char *D_80140018;
extern u_char *D_8014001C;
extern u_char *D_80140020;
extern u_char *D_80140024;
extern u_char *D_80140028;
extern u_char *D_8014002C;
extern u_char *D_80140030;
extern u_char *D_80140038;
extern u_char *D_8014003C;
extern u_char *D_80140040;
extern u_char *D_80140044;
extern u_char *D_80140048;
extern u_char *D_8014004C;
extern u_char *D_80140050;
extern u_char *D_80140054;

/* Once the run has been consumed its memory becomes the battle's own buffer. */
#define BTL_LOAD_BUF ((u_char *)0x80140000)

/* Where the save block's actor flags start, which the battle is pointed at
   before anything else. */
#define BTL_SAVE_FLAGS ((u_char *)0x801F2678)

/* The three entries that are read more than once, which the routine holds as
   addresses rather than fetching twice. */
#define BTL_LOAD_VH_END  ((u_char **)0x80140008)
#define BTL_LOAD_SEQ_END ((u_char **)0x8014000C)
#define BTL_LOAD_BG      ((u_char **)0x80140034)

/* The five sound banks the battle opens its fixed slots from. Only four are
   used: bank nought is the battle's own music, which is why its three buffers
   are the ones patched from what the loader read, and banks two to four are
   built into the overlay with only their body pointer left to fill in. Bank one
   is all zeroes and nothing opens it. */
extern BtlSoundBank g_btl_open_banks[];

/* Which slot each of them is opened as. */
#define BTL_OPEN_BGM   0
#define BTL_OPEN_HIT   1
#define BTL_OPEN_VOICE 2
#define BTL_OPEN_SCENE 5

/* And which bank each slot takes. */
#define BTL_BANK_BGM   0
#define BTL_BANK_HIT   2
#define BTL_BANK_VOICE 3
#define BTL_BANK_SCENE 4

/* What the sound debug print reads - three bytes out of each of the five
   party records, and a byte out of the table each record indexes. */
extern u_char D_801F1C0A, D_801F1C23, D_801F1C24;
extern u_char D_801F1C6A, D_801F1C83, D_801F1C84;
extern u_char D_801F1CCA, D_801F1CE3, D_801F1CE4;
extern u_char D_801F1D2A, D_801F1D43, D_801F1D44;
extern u_char D_801F1D8A, D_801F1DA3, D_801F1DA4;
/* The record each party member's sound is described by, reached by address:
   the assembler expands a numeric base with an index the other way round from a
   named one, and the image has the numeric form. */
#define BTL_SOUND_REC_SHIFT 6
#define BtlSoundRec(idx)     (*(u_char *)(0x801F1DC4 + ((idx) << BTL_SOUND_REC_SHIFT)))

extern u_char *D_800F6130;

extern int     D_800F49D8;
extern int     D_800F4BA0;
extern int     D_800F4BAC;
extern u_char *D_800F4ABC;
extern u_char *D_800F4AC4;
extern u_char *D_800F5CAC;
extern u_char *D_800F5D68;
extern u_char *D_800F5D6C;
extern u_char *g_btl_gfx_next;
extern u_char *g_btl_hud_packed;
extern u_char *g_btl_frame_packed;
extern u_char *g_btl_box_pack;
extern u_char *g_btl_panel_pack;
extern u_char *g_btl_tim_buf;
extern int     g_btl_enemy_gfx_base;
extern int     g_btl_gfx_sector;
extern int     g_btl_slot_sound_base;
extern int     g_btl_sound_base;
extern short   D_800F4C86;
extern u_char  D_800F4DC8[];
extern long    D_800F4BF0[];
extern u_short D_800F4D68;
extern u_short D_800F4D6C;
extern u_short D_800F4D94;

/* The moon's two tables, and the pair of pointers the battle keeps out of
   them for the fight it is about to run. */
extern u_char *D_800DAD38;
extern u_char *D_800DAD98;
extern u_char *D_800DF524[];
extern u_char *D_800DF564[];

/* The two the control scheme picks between. */
extern u_char *D_800DC12C;
extern u_char *D_800DC228;
extern u_char  D_800DC074[];
extern u_char  D_800DC07C[];
extern u_char  D_800DC194[];
extern u_char  D_800DC1D4[];

extern u_char D_800E010C[];
extern u_char D_800E1FD8[];
extern u_char D_800CFAAC[];
extern u_char D_800CFABC[];

/* The access lamp and the three objects beside it. */
extern BtlObj *D_800EC604;
extern BtlObj *D_800EC608;
extern BtlObj *D_800EC60C;
extern BtlObj *D_800EC610;

extern u_char g_btl_mesh_show;
extern u_char g_btl_arena_show;
extern u_char g_btl_intro_step;
extern u_char g_btl_debug;
extern u_char g_btl_formation[];

extern BtlObjDef g_btl_shadow_defs[];

/* Where the battle's own palette line is put in VRAM. */
extern RECT g_btl_clut_line;

/* The moon phase the field was left on. */
extern u_char g_moon;

extern void BtlInitGraphics(void);
extern void BtlInitObjects(void);
extern void BtlSoundInit(void);
extern void BtlDebugWait(const char *what);
extern void BtlDebugWaitArgs(const char *fmt, int a, int b, int c);
extern void BtlClockTick(void);
extern void BtlUploadPackedTim(u_char *src, int tx, int ty, int cx, int cy,
                               int n);
extern void BtlUploadTim(u_char *src, int tx, int ty, int cx, int cy, int n);
extern void BtlBuildMesh(void);
extern void BtlTakeParty(void);
extern void BtlLoadPersonas(void);
extern void BtlPlaceFormation(void);
extern void BtlFormationCloseUp(void);
extern void BtlSpawnParty(void);
extern void BtlSpawnEnemies(int encounter);
extern void BtlShowAilmentMarks(int on);
extern void BtlAverageSides(void);
extern void BtlUnpack(u_char *dest, const u_char *src);
extern void BtlSpawnMarkers(void);
extern void BtlPickSpawn(void);
extern int  BtlBindGfx(u_int kind, int index, u_char **image);

/* The one-shade CLUT the battle lays over the stage buffer before uploading
   it: entry nought transparent, every other entry the same grey. */
#define BTL_CLUT_ENTRIES 0x100
#define BTL_CLUT_FILL    0x8C63

/* A g_map_id at or past this is not a battle field, so the default stands in
   for it. */
#define BTL_FIELD_MAX 0x168
#define BTL_FIELD_DEF 0x10

/* Steps the message box straight to its last phase and puts the text page
   back, which is how a stage takes a box away without waiting for it. */
void BtlBoxDismiss(void)
{
    g_btl_box_step  = 4;
    g_btl_text_page = 1;
}

#ifdef NON_MATCHING
static __inline__ int BtlEntryEncounter(void)
{
    return g_btl_encounter;
}

/* The remaining instruction differences are the encounter-check copies,
   the first texture upload's scheduling, and the final stage-table address.
   odiff also reports the raw addresses and unowned string section as aliases. */
void ovl_btlp_entry(void)
{
    long     pos[4];
    CdlFILE  file[2];
    u_char *image[6];
    u_char  *bg;
    u_char **bank;
    u_char  *a;
    u_char  *b;
    u_char  *f;
    u_char  *cell;
    u_short *clut;
    short   *p;
    long    *slot;
    u_char   t;
    u_char   u;
    u_char   orders;
    u_char **seq_end;
    BtlObj  *o;
    BtlObj  *lamp;
    int      i;
    int      enc;
    int      full_moon;
    short    shade;
    void   (*stage)(void);
    void  (**stages)(void);

    image[0]   = BTL_WORK_A;
    D_800F6130 = BTL_SAVE_FLAGS;
    BtlPadRead();

    /* Both schemes are written out in full rather than sharing the part they
       agree on - the image stores all fourteen either way. */
    if (g_pad_config != 0) {
        g_btl_key_up      = 0x1000;
        g_btl_key_down    = 0x4000;
        g_btl_key_left    = 0x8000;
        g_btl_key_right   = 0x2000;
        g_btl_help_key    = 0x10;
        g_btl_key_cancel  = 0x1;
        g_btl_key_abort   = 0x800;
        g_btl_key_confirm = 0x4;
        g_btl_key_select  = 0x100;
        g_btl_key_square  = 0x80;
        g_btl_key_r1      = 0x8;
        g_btl_key_r2      = 0x2;
        g_btl_key_page    = 0x20;
        g_btl_key_end     = 0x40;

        D_800DC12C = D_800DC07C;
        D_800DC228 = D_800DC1D4;
    } else {
        g_btl_key_up      = 0x1000;
        g_btl_key_down    = 0x4000;
        g_btl_key_left    = 0x8000;
        g_btl_key_right   = 0x2000;
        g_btl_help_key    = 0x10;
        g_btl_key_cancel  = 0x40;
        g_btl_key_abort   = 0x80;
        g_btl_key_confirm = 0x20;
        g_btl_key_select  = 0x4;
        g_btl_key_square  = 0x1;
        g_btl_key_r1      = 0x8;
        g_btl_key_r2      = 0x2;
        g_btl_key_page    = 0x100;
        g_btl_key_end     = 0x800;

        D_800DC12C = D_800DC074;
        D_800DC228 = D_800DC194;
    }

    D_800EE618      = g_map_id;
    g_btl_bgm_index = g_map_unk4;
    if (D_800EE618 >= BTL_FIELD_MAX) {
        D_800EE618 = BTL_FIELD_DEF;
    }

    if (D_8004E267 != 0) {
        D_800EE618 = g_btl_encounter;
    } else {
        g_btl_encounter = D_800EE618;
    }

    g_btl_battle_kind = g_map_room;
    if (g_btl_debug_party_only != 0) {
        g_btl_battle_kind = 1;
    }

    enc = BtlEntryEncounter();
    if (enc < 9 || (u_int)(enc - 0x11) < 2) {
        g_btl_place_party = 1;
        g_btl_battle_kind = 0;
    }

    enc = BtlEntryEncounter();
    if (enc < 5 || (u_int)(enc - 6) < 2 || (u_int)(enc - 0x11) < 2) {
        D_800CCA2D = 1;
    }

    if (g_btl_encounter < 0x23 && D_800CCA2D == 0) {
        g_btl_no_escape = 1;
    }

    BtlInitGraphics();
    if (g_btl_debug != 0) {
        BtlDebugWait("GPU INIT\n");
    }

    BtlInitObjects();
    if (g_btl_debug != 0) {
        BtlDebugWait("PRIM INIT\n");
    }

    BtlSoundInit();
    if (g_btl_debug != 0) {
        BtlDebugWait("SND INIT\n");
        if (g_btl_debug != 0) {
            BtlDebugWaitArgs("%02x %02x %02x\n", D_801F1C0A, D_801F1C23,
                             BtlSoundRec(D_801F1C24));
            BtlDebugWaitArgs("%02x %02x %02x\n", D_801F1C6A, D_801F1C83,
                             BtlSoundRec(D_801F1C84));
            BtlDebugWaitArgs("%02x %02x %02x\n", D_801F1CCA, D_801F1CE3,
                             BtlSoundRec(D_801F1CE4));
            BtlDebugWaitArgs("%02x %02x %02x\n", D_801F1D2A, D_801F1D43,
                             BtlSoundRec(D_801F1D44));
            BtlDebugWaitArgs("%02x %02x %02x\n", D_801F1D8A, D_801F1DA3,
                             BtlSoundRec(D_801F1DA4));
        }
    }

    CdSearchFileLoc(file, "\\B\\D.BIN;1");
    g_btl_enemy_gfx_base = CdPosToInt(&file[0].pos);
    CdSearchFileLoc(file, "\\B\\P.BIN;1");
    g_btl_gfx_sector = CdPosToInt(&file[0].pos);
    CdSearchFileLoc(file, "\\B\\G.BIN;1");
    D_800F4BAC = CdPosToInt(&file[0].pos);
    CdSearchFileLoc(file, "\\B\\M.BIN;1");
    D_800F4BA0 = CdPosToInt(&file[0].pos);
    CdSearchFileLoc(file, "\\B\\DS.BIN;1");
    g_btl_pack_bank_base = CdPosToInt(&file[0].pos);
    CdSearchFileLoc(file, "\\B\\DD.BIN;1");
    g_btl_slot_sound_base = CdPosToInt(&file[0].pos);
    CdSearchFileLoc(file, "\\B\\PS.BIN;1");
    g_btl_pack_base = CdPosToInt(&file[0].pos);
    CdSearchFileLoc(file, "\\B\\PD.BIN;1");
    g_btl_sound_base = CdPosToInt(&file[0].pos);
    CdSearchFileLoc(file, "\\B\\SS.BIN;1");
    D_800F49D8 = CdPosToInt(&file[0].pos);
    CdSearchFileLoc(file, "\\B\\MS.BIN;1");
    g_btl_voice_base = CdPosToInt(&file[0].pos);

    /* The three buffers are consecutive words, and the routine works from the
       middle one: the bank itself is a word below what it keeps the address
       of, which is where the negative displacements come from. */
    seq_end = BTL_LOAD_SEQ_END;
    bank  = &g_btl_open_banks[0].vb;
    g_btl_open_banks[0].vh   = BTL_WORK_A;
    g_btl_open_banks[0].seq  = (u_long *)BTL_WORK_B;
    *bank = g_load_stage;
    g_btl_open_banks[2].vb   = *seq_end;
    g_btl_open_banks[3].vb   = D_80140010;
    g_btl_open_banks[4].vb   = D_80140014;
    g_btl_open_banks[0].nsep = g_btl_bgm_kinds[g_btl_bgm_index];
    memmove(BTL_WORK_A, D_80140004, *BTL_LOAD_VH_END - D_80140004);
    memmove(BTL_WORK_B, *BTL_LOAD_VH_END,
            *seq_end - *BTL_LOAD_VH_END);

    if (g_btl_debug != 0) {
        BtlDebugWaitArgs("%02x %02x %02x\n", bank[-1][0], bank[-1][1],
                         bank[-1][2]);
    }

    BtlSoundOpen(g_btl_open_banks, BTL_OPEN_BGM, BTL_BANK_BGM);
    BtlSoundOpen(g_btl_open_banks, BTL_OPEN_HIT, BTL_BANK_HIT);
    BtlSoundOpen(g_btl_open_banks, BTL_OPEN_VOICE, BTL_BANK_VOICE);
    BtlSoundOpen(g_btl_open_banks, BTL_OPEN_SCENE, BTL_BANK_SCENE);
    if (g_btl_debug != 0) {
        BtlDebugWait("SND TRANSFER\n");
    }

    VSyncCallback(BtlClockTick);
    if (g_btl_debug != 0) {
        BtlDebugWait("CALL BACK SET\n");
    }

    enc = (int)g_load_stage;
    g_btl_tim_buf = (u_char *)enc;
    BtlUploadPackedTim(D_80140018, 0x1F, 0x20, 0, 0, 7);
    BtlUploadPackedTim(D_8014001C, 0x1F, 0x27, 0, 0x80, 5);
    BtlUploadPackedTim(D_80140020, 0x19, 0x1E, 0, 0, 1);
    if (g_btl_debug == 0) {
        BtlUploadPackedTim(D_80140024, 0x18, 0x17, 2, 0x80, 1);
    }
    D_800F4D6C = D_800F4D94;
    BtlUploadPackedTim(D_80140028, 0x15, 0x15, 0, 0, 1);
    BtlUploadPackedTim(D_8014002C, 0x16, 0x16, 0, 0, 1);
    if (g_btl_debug == 0) {
        BtlUploadPackedTim(D_80140030, 0x18, 0x40, 0, 0, 0x20);
    }
    if (g_btl_debug != 0) {
        BtlDebugWait("TEXTURE SET\n");
    }

    bg = BTL_BG_FIELD;
    if (g_state_prev == 0) {
        bg = BTL_BG_DNG;
    }
    memmove(bg, *BTL_LOAD_BG, D_80140054 - *BTL_LOAD_BG);
    D_800F5D68         = bg;
    D_800F5D6C         = bg + (int)D_80140038 - (int)*BTL_LOAD_BG;
    D_800F4ABC         = bg + (int)D_8014003C - (int)*BTL_LOAD_BG;
    D_800F4AC4         = bg + (int)D_80140040 - (int)*BTL_LOAD_BG;
    g_btl_hud_packed   = bg + (int)D_80140044 - (int)*BTL_LOAD_BG;
    g_btl_frame_packed = bg + (int)D_80140048 - (int)*BTL_LOAD_BG;
    g_btl_box_pack     = bg + (int)D_8014004C - (int)*BTL_LOAD_BG;
    g_btl_panel_pack   = bg + (int)D_80140050 - (int)*BTL_LOAD_BG;

    BtlBuildMesh();
    if (g_btl_debug != 0) {
        BtlDebugWait("BG INIT\n");
    }

    /* The buffer is turned into a palette line in place, then uploaded. The
       run walks a pointer of its own rather than indexing off the first. */
    i       = 1;
    clut    = (u_short *)BTL_LOAD_BUF;
    clut[0] = 0;
    shade   = BTL_CLUT_FILL;
    p       = (short *)(clut + 1);
    do {
        *p = shade;
        p++;
        i++;
    } while (i < BTL_CLUT_ENTRIES);
    LoadImage(&g_btl_clut_line, (u_long *)clut);
    i          = 0;
    D_800F4C86 = GetClut(g_btl_clut_line.x, g_btl_clut_line.y);

    cell = D_800F4DC8;
    do {
        i++;
        *cell = 0;
        cell++;
        g_btl_debug_grid_cells[0] = 0;
    } while (i < 0x4B);

    i    = 7;
    slot = D_800F4BF0;
    do {
        *slot = 0;
        slot--;
        i--;
    } while (i >= 0);

    full_moon = 8;
    BtlTakeParty();
    BtlLoadPersonas();
    BtlPlaceFormation();
    if (g_btl_debug != 0) {
        BtlDebugWait("WORK COPY\n");
    }

    g_btl_moon = g_moon;
    /* A new moon has the party fight on its own orders and a full moon takes
       them away altogether; anything between is the ordinary set. */
    if (g_btl_moon == 0) goto new_moon;
    if (g_btl_moon == full_moon) goto full_moon_orders;
    orders = 1;
    goto store_moon_orders;
new_moon:
    orders = 2;
    goto store_moon_orders;
full_moon_orders:
    g_btl_ai_set = 0;
    goto moon_orders_done;
store_moon_orders:
    g_btl_ai_set = orders;
moon_orders_done:

    D_800DAD38 = *(u_char **)((u_char *)D_800DF524 + g_btl_moon * 4);
    D_800DAD98 = *(u_char **)((u_char *)D_800DF564 + g_btl_moon * 4);

    /* Where the next graphics image is read to, which the field and the dungeon
       each have their own buffer for. The copy below is taken from it rather
       than the other way round. */
    if (g_state_prev == 0) {
        g_btl_gfx_next = BTL_BG_FIELD;
    } else {
        g_btl_gfx_next = BTL_BG_DNG;
    }

    g_btl_tpage[0]  = 0x40;
    D_800F4D68      = 0;
    pos[0]          = 0;
    pos[1]          = 0;
    pos[2]          = 0;
    D_800F5CAC      = g_btl_gfx_next;
    g_btl_draw_dist = g_btl_screen_dist;

    o = BtlObjAlloc(g_btl_shadow_defs, 0, NULL, 6, 0, pos, 0, 0);
    g_btl_intro_obj = o;
    o->rgb[0]       = 0xFF;
    o->rgb[1]       = 0xFF;
    o->rgb[2]       = 0xFF;
    o->rgb_to[0]      = 0;
    o->rgb_to[1]      = 0;
    o->rgb_to[2]      = 0;
    o->fade        = 2;

    pos[0] = 0x1000000;
    pos[1] = 0x200000;
    pos[2] = 0;
    D_800EC608 =
        BtlObjAlloc(g_btl_shadow_defs, 0, NULL, 2, 0xF, pos, 0x1F, 0x20);
    lamp = BtlObjAlloc(g_btl_shadow_defs, 0, NULL, 2, 0xE, pos, 0x1F, 0x20);
    lamp->rgb_to[0]       = 0x30;
    lamp->rgb_to[1]       = 0x30;
    lamp->rgb_to[2]       = 0x30;
    D_800EC604          = lamp;
    D_800EC608->rgb_to[0] = 0xFF;
    D_800EC608->rgb_to[1] = 0xFF;
    D_800EC608->rgb_to[2] = 0xFF;
    lamp->fade         = 0xFF;
    D_800EC608->fade   = 0xFF;

    pos[1] = 0x380000;
    D_800EC60C =
        BtlObjAlloc(g_btl_shadow_defs, 1, NULL, 2, 0x11, pos, 0x1F, 0x20);
    pos[1] = 0x680000;
    D_800EC610 =
        BtlObjAlloc(g_btl_shadow_defs, 1, NULL, 2, 0x16, pos, 0x1F, 0x20);
    if (g_btl_debug != 0) {
        BtlDebugWait("ACCESS LAMP SET\n");
    }

    g_btl_intro_obj->attr &= ~0x40000000;
    BtlDrawFrame();
    BtlDrawFrame();

    g_btl_mesh_show  = 1;
    g_btl_arena_show = 1;
    SetDispMask(1);

    image[0] = D_800E1FD8;
    BtlBindGfx(4, 0, image);

    /* A pincer fight starts the party the other way round, so the near half of
       the formation is swapped end for end with the far half. */
    if (g_btl_battle_kind == 2) {
        f = g_btl_formation;
        b = f + 0x18;
        do { a = f; } while (0);
        do {
            t = *b;
            u = *a;
            *a = t;
            a++;
            *b = u;
            b--;
        } while ((int)a < (int)f + 0xD);
        BtlFormationCloseUp();
    }

    BtlSpawnParty();
    if (g_btl_debug != 0) {
        BtlDebugWait("PARTY SET\n");
    }

    BtlSpawnEnemies(D_800EE618);
    if (g_btl_debug != 0) {
        BtlDebugWait("DEVIL SET\n");
    }

    BtlShowAilmentMarks(1);
    BtlAverageSides();
    BtlUnpack(BTL_LOAD_BUF, D_800E010C);
    BtlUploadTim(BTL_LOAD_BUF, 0xD, 0x1D, 0, 0x80, 1);
    DrawSync(0);

    /* An ambush and a pincer each open with their own line; anything else opens
       with none. The two calls are written out rather than sharing one - the
       image merges only the call itself. */
    switch (g_btl_battle_kind) {
    case 1:
        BtlOpenMessage(1, 1, D_800CFAAC, 0x10, 0x10);
        break;

    case 2:
        BtlOpenMessage(1, 2, D_800CFABC, 0x10, 0x10);
        break;
    }
    g_btl_intro_step = 4;

    BtlSpawnMarkers();
    BtlPickSpawn();
    BtlBuildMarkers();
    if (g_btl_debug != 0) {
        BtlDebugWait("INIT OK!!\n");
    }

    stages = g_btl_stages;

next_stage:
    enc = g_btl_stage;
    enc = (u_char)enc * sizeof(stage) + (int)stages;
    stage = *(void (**)(void))enc;
    if (stage != NULL) {
        g_btl_step = 0;
        stage();
        BtlDrawFrame();
        goto next_stage;
    }
}

#else
INCLUDE_ASM("btlp/nonmatchings/entry", ovl_btlp_entry);
#endif

/* Nothing in the image reaches this - no jal to it and no word holding its
   address - so it is whatever was left of a routine the battle stopped needing.
   It has to be here all the same: the eight bytes sit between the entry point
   and BtlDrawFrame. */
void BtlEntryStub(void)
{
}
