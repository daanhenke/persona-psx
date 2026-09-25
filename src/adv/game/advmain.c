/* Persona 1 (JP) - the ADV overlay's entry: one visit to a room.  ADV only.
 *   0x8007D9EC ovl_adv_entry
 *
 * The overlay is loaded to run one room and hands control back when the
 * player leaves it. In order:
 *
 *  - A new game (no map yet) builds the first character, clears the saved
 *    formations, the Persona slots and every Persona record, and sets two
 *    starting figures and stereo.
 *  - The room's state is reset and the two ordering tables set up.
 *  - The scene is loaded. Enter mode 4 only reloads the event room data
 *    (EV0n.BGD, by room kind) over what is there. Otherwise the packed scene
 *    file, already read to 0x80180000, is unpacked to 0x80118000: four system
 *    images go to VRAM and the scene pack proper, 0xFFF8 bytes at +0xA0008,
 *    is copied to 0x80100000. The pack's music is faded in if it names any,
 *    then its event background and MES images are loaded.
 *  - The command bar (ADVCMD.BIN) is loaded and slides down, and the arrival
 *    script runs: where a script that left the room last time stopped, if
 *    it did, otherwise the scene's own.
 *  - The field runs until a script leaves the room or the field itself
 *    exits. The leave code becomes the next enter mode; the bar slides back
 *    up, the next overlay is preloaded, the music is faded out and the sound
 *    banks closed.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/adv/scene.h>
#include <persona/adv/room.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>
#include <persona/common/imageanim.h>

#define NONE 0xFFFF

/* The save-game option bytes, reached from four before g_options. */
#define g_cfg ((u_char *)0x801F2AC4)

#define g_party_at         ((u_char *)0x801F256C)
#define g_formation_preset ((u_char *)0x801F2584)
#define g_persona_slots    ((u_char *)0x801F2574)
#define g_adv_room         (*(u_char *)0x801F5355)
#define g_script_resuming             (*(u_char *)0x801F1B88)
#define g_script_resume    (*(u_char **)0x801F1B84)
#define g_bgm_vab          (*(u_short *)0x801F5360)
#define g_seq_handle       ((short *)0x801F537C)
#define g_pad_config       (*(u_char *)0x801F2AC7)
/* Leave code 1 reaches these two by address, where the rest use the names. */
#define MAP_ID_AT          (*(u_short *)0x801F5350)
#define GOTO_MAP_AT        (*(u_short *)0x801F534C)

/* The packed scene file as the loader left it, and where it unpacks. */
#define SCENE_PACKED  ((u_char *)0x80180000)
#define SCENE_AT      ((u_long *)0x80118000)
#define SCENE_PACK_AT ((u_char *)0x801B8008)
#define SCENE_PACK_LEN 0xFFF8
#define ROOM_DATA_AT  ((void *)0x800EB780)

#define MEMBER(at, n) ((u_long *)((u_char *)(at) + (at)[n]))

/* What the room is entered as, and left as. */
#define ENTER_NONE  (-1)
#define ENTER_DNG   0
#define ENTER_FIELD   1
#define ENTER_S2D   2
#define ENTER_ADV   3
#define ENTER_EVENT 4
#define ENTER_NAME  5
#define ENTER_6     6

#define BAR_SLOT  0x29
#define BAR_SLOT2 0x2A

extern short  g_adv_enter_mode;
extern short  g_adv_field_state;
extern int    g_state_prev;
extern int    g_state_next;
extern u_short g_map_id;
extern u_short g_script_534C;
extern short  g_script_15C2;
extern short  g_script_97C;
extern short  g_bgm_ready;
extern short  g_field_exit;
extern u_char g_script_leave;
extern u_char g_adv_loading;
extern u_char g_adv_char_seq;
extern u_char g_actor_dim;
extern u_char g_cam_actor;
extern u_char g_fade_sprites;
extern u_char g_advcmd_loaded;
extern u_char g_options[];
/* Two figures a new game starts from; not worked out yet. */
extern int    g_29B0;
extern int    g_29B4;
extern u_short g_prev_map;
extern int    g_exp_carry;
extern int    g_image_queue_count;
extern int    g_ot_index;
extern int    g_pad_held[];
extern int    g_pad_pressed[];
extern GsOT   g_ot[];
extern AdvScene *g_adv_scene;
extern volatile int g_cd_busy;
extern u_char g_pdata_index[];
extern char   str_cd0_p_data[];
extern char   str_adv_ev01_bgd[];
extern char   str_adv_ev02_bgd[];
extern char   str_adv_ev03_bgd[];

/* Room state cleared on the way in. */
extern u_char g_BC5B8, g_BC5BC, g_BC5C4, g_BC204, g_BC5C8;
extern int    g_BB94C, g_BB998;

extern void func_80033A50(int a, int b, int c, int d);
extern void func_80034850(u_long *base);
extern void CharJoin(u_char chr, u_char key, u_char level);
extern void CharSetLevelExp(u_char level, u_char slot);
extern void VramClearRect(int x, int y, int w, int h);
extern void LoadFileToAddr(const char *name, void *dest);
extern void LoadFileToAddrAsync(const char *name, void *dest);
extern void AdvRoomRebuild(void);
extern void AdvResetBanks(void);
extern void AdvCloseBanks(void);
extern void AdvSilenceBgm(void);
extern void SlotClearAll(void);
extern void FadeBlackout(void);
extern void FadeStepDown(u_char step, u_char floor);
extern void FadeStepUp(u_char step, u_char limit);
extern int  FadeSpritesStep(short step, short limit);
extern void AdvUnpack(u_char *src, u_char *dst, u_int size);
extern void TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void bcopy(void *src, void *dst, int len);
extern int  EventFlagGet(short id);
extern void AdvLoadBgm();  /* called without a prototype here */
extern void AdvLoadEventBg(void);
extern void AdvPickSceneByFlags(void);
extern void ViewShakeStop(void);
extern void func_80088B8C(void);
extern void PadLoadBindings(u_char config);
extern void PadSetPageButtons(u_char config);
extern void AdvFieldEnter(void);
extern void AdvFieldTick(void);
extern void func_8008FC78(u_char n);
extern void AdvPickEffect(void);
extern void AdvEffectSetupSlots(void);
extern void AdvRunFrame(void);
extern int  AdvRunScript(u_char *s);
extern void AdvFadeUpBlocking(short step, short limit);
extern void AdvFadeDownBlocking(short step, short floor);
extern void MapRevealScene(void);
extern void SlotSetPos(u_char slot, int attr, short x, short y);
extern void func_8007E650(void);
extern void D_8007E618(void);
extern void PreloadDng(void);
extern void PreloadS2d(void);
extern void PreloadBtlField(void);
extern void PreloadName(void);
extern void PreloadAdv(void);

#ifdef NON_MATCHING
void ovl_adv_entry(void)
{
    u_char *cfg = g_cfg;
    u_char *resume;
    u_int   i;
    int     y;
    u_int   n;

    VSync(0);
    GsInitGraph(0x140, 0xF0, 0x100, 0, 0);
    func_80033A50(0, 0, 0, 0xF0);

    resume = (u_char *)-1;
    if (g_map_id == 0) {
        /* A new game. */
        g_party_at[0] = 0;
        g_party_at[1] = 0xFF;
        g_party_at[2] = 0xFF;
        g_party_at[3] = 0xFF;
        g_party_at[4] = 0xFF;
        CharJoin(0, 1, 5);
        CharSetLevelExp(5, 0);
        g_chars[0].unk1C = ExpToLevel(4, 0, 0);
        for (i = 0; i < 0xE1; i++) {
            g_formation_preset[i] = 0xFF;
        }
        for (i = 0; i < 16; i++) {
            g_persona_slots[i] = 0xFF;
        }
        for (i = 0; i < 31; i++) {
            g_personas[i].key = 0;
            g_personas[i].owner = 0xFF;
        }
        g_29B0 = 1000000;
        g_29B4 = 300000;
        g_options[0] = 1;
    }
    if (g_adv_room == 8) {
        g_adv_room = 7;
        for (i = 0; i < 5; i++) {
            g_chars[i].unk5D = 0;
        }
    }
    if (g_script_resuming) {
        resume = g_script_resume;
    }

    g_script_leave = 0;
    g_script_97C = 0x100;
    g_BC5B8 = 0;
    g_BC5BC = 0;
    g_BC5C4 = 0;
    g_BC204 = 0;
    g_BC5C8 = 0;
    g_BB94C = 0;
    g_image_queue_count = 0;
    g_BB998 = 0;
    g_actor_dim = 0;
    g_adv_loading = 0;
    g_adv_char_seq = 0;
    g_adv_scene = (AdvScene *)0x80100034;
    g_adv_enter_mode = g_state_prev;
    g_prev_map = g_map_id;

    SetDispMask(0);
    g_ot[1].length = 11;
    g_ot[0].length = 11;
    g_ot[0].org = (GsOT_TAG *)0x800D6000;
    g_ot[1].org = (GsOT_TAG *)0x800D9000;
    g_ot_index = GsGetActiveBuff();
    func_80034850((u_long *)(0x800C0000 + g_ot_index * 0xB000));
    GsClearOt(0, 0, &g_ot[0]);
    GsClearOt(0, 0, &g_ot[1]);
    VramClearRect(0, 0, 0x140, 0x1DF);

    switch (g_adv_enter_mode) {
    case ENTER_EVENT:
        /* Only the room data changes. */
        switch (g_adv_scene->kind) {
        case 2:
            LoadFileToAddr(str_adv_ev01_bgd, ROOM_DATA_AT);
            break;
        case 3:
            LoadFileToAddr(str_adv_ev02_bgd, ROOM_DATA_AT);
            break;
        case 4:
            LoadFileToAddr(str_adv_ev03_bgd, ROOM_DATA_AT);
            break;
        }
        AdvRoomRebuild();
        AdvResetBanks();
        goto loaded;
    case ENTER_FIELD:
        SsEnd();
        SsQuit();
        SsInit();
        g_bgm_ready = 1;
        break;
    }

    SlotClearAll();
    FadeBlackout();
    ImageAnimStopAll();
    AdvUnpack(SCENE_PACKED, (u_char *)SCENE_AT, -1);
    TimQueueAt(MEMBER(SCENE_AT, 3), 0x394, 0x60, 0, 0x1E4);
    TimQueueAt(MEMBER(SCENE_AT, 2), 0x388, 8, 0, 0x1E4);
    TimQueueAt(MEMBER(SCENE_AT, 1), 0x380, 0, 0, 0x1E2);
    TimQueueAt(MEMBER(SCENE_AT, 0), 0x140, 0, 0x110, 0x1E0);
    FlushImageUploads();
    DrawSync(0);
    bcopy(SCENE_PACK_AT, g_adv_pack, SCENE_PACK_LEN);

    g_bgm_ready = 0;
    if (g_adv_pack->bgm.flag == NONE || !(u_char)EventFlagGet(g_adv_pack->bgm.flag)) {
        i = g_adv_pack->bgm.id;
    } else {
        i = g_adv_pack->bgm.alt;
    }
    if (i != NONE) {
        for (y = 0x7F, n = 0; n < 0x20; n++, y -= 4) {
            SsSetMVol(y, y);
            VSync(0);
        }
        SsSetMVol(0, 0);
        AdvLoadBgm(i);
    }
    AdvResetBanks();
    AdvLoadEventBg();
    AdvPickSceneByFlags();
    FlushImageUploads();
    DrawSync(0);

loaded:
    ViewShakeStop();
    g_cam_actor = 0;
    AdvRoomBgInit();
    func_80088B8C();
    PadLoadBindings(g_pad_config);
    PadSetPageButtons(g_pad_config);
    g_field_exit = 0;
    g_adv_field_state = 1;
    g_BB94C = 0;
    g_pad_held[0] = 0;
    g_pad_pressed[0] = 0;
    AdvFieldEnter();
    func_8008FC78(cfg[2]);
    g_advcmd_loaded = 1;
    LoadFileToAddrAsync("\\ADV\\ADVCMD.BIN;1", SCENE_AT);
    AdvPickEffect();
    AdvEffectSetupSlots();
    FadeStepDown(0, 0);
    if (g_map_id != 3 || (u_char)EventFlagGet(4)) {
        g_fade_sprites = 2;
        while (!FadeSpritesStep(2, 0x80)) {
            FadeStepUp(2, 0x80);
            AdvRunFrame();
        }
        FadeStepUp(2, 0x80);
        AdvRunFrame();
        g_fade_sprites = 0;
    }
    while (g_cd_busy != -1) {
        AdvRunFrame();
    }
    TimQueueAt(MEMBER(SCENE_AT, 1), 0x380, 0x1C8, 0x100, 0x1F8);
    TimQueueAt(MEMBER(SCENE_AT, 0), 0x380, 0x100, 0x3C0, 0x1A0);
    AdvRunFrame();
    for (i = 0x10; i != -2; i--) {
        y = 0xC - i * 2;
        SlotSetPos(BAR_SLOT, 0x35, 0xE0, y);
        SlotSetPos(BAR_SLOT2, 0x34, 0xE3, y);
        AdvRunFrame();
    }
    AdvRunFrame();
    g_bgm_vab = NONE;

    if (resume != (u_char *)-1) {
        g_script_leave = AdvRunScript(resume);
    } else if (g_adv_scene->arrive != (u_char *)-1) {
        g_script_leave = AdvRunScript(g_adv_scene->arrive);
    }
    g_script_resuming = 0;
    TimQueueAt(MEMBER(SCENE_AT, 1), 0x380, 0x1C8, 0x100, 0x1F8);
    TimQueueAt(MEMBER(SCENE_AT, 0), 0x380, 0x100, 0x3C0, 0x1A0);
    AdvFadeUpBlocking(8, 0x80);
    MapRevealScene();

    /* The field, until a script leaves the room or the field exits. */
    while (g_field_exit != 0xFF) {
        AdvRunFrame();
        if (g_script_leave) {
            switch (g_script_leave) {
            case 4:
                g_adv_enter_mode = ENTER_DNG;
                goto leave;
            case 3:
                g_adv_enter_mode = ENTER_S2D;
                goto leave;
            case 2:
                g_adv_enter_mode = ENTER_FIELD;
                g_map_id = g_script_534C;
                goto leave;
            case 1:
                if (MAP_ID_AT == 0) {
                    g_adv_enter_mode = ENTER_6;
                    g_script_15C2 = 0x11;
                    goto leave;
                }
                if (MAP_ID_AT == 0x1CF) {
                    goto none;
                }
                g_adv_enter_mode = ENTER_ADV;
                if (GOTO_MAP_AT != 0) {
                    MAP_ID_AT = GOTO_MAP_AT;
                }
                break;
            case 5:
                g_adv_enter_mode = ENTER_EVENT;
                goto leave;
            case 6:
                g_adv_enter_mode = ENTER_6;
                goto leave;
            case 7:
            none:
                g_adv_enter_mode = ENTER_NONE;
                goto leave;
            }
            break;
        }
        if (g_field_exit == 0) {
            AdvFieldTick();
        }
    }

leave:
    /* The command bar slides back up. */
    for (i = -1; i != 0x14; i++) {
        y = 0xC - i * 2;
        SlotSetPos(BAR_SLOT, 0x35, 0xE0, y);
        SlotSetPos(BAR_SLOT2, 0x34, 0xE3, y);
        AdvRunFrame();
    }
    if (g_adv_enter_mode != g_state_prev || g_adv_enter_mode == ENTER_FIELD) {
        g_bgm_ready = 1;
    }

    switch (g_adv_enter_mode) {
    case ENTER_DNG:
        func_8007E650();
        PreloadDng();
    case ENTER_6:
    common:
        D_8007E618();
        break;
    case ENTER_S2D:
        func_8007E650();
        PreloadS2d();
        goto common;
    case ENTER_FIELD:
        func_8007E650();
        PreloadBtlField();
        goto common;
    case ENTER_NAME:
        PreloadName();
        goto common;
    case ENTER_ADV:
        func_8007E650();
        PreloadAdv();
        break;
    case ENTER_EVENT:
        if (!(u_char)EventFlagGet(300)) {
            g_exp_carry = g_chars[0].unk10;
        }
        i = g_pdata_index[g_script_534C];
        LoadFileToAddrAsync(&str_cd0_p_data[i * 20], SCENE_AT);
        break;
    }
    g_state_next = g_adv_enter_mode;
    AdvFadeDownBlocking(2, 0);
    if (g_bgm_vab != NONE) {
        SsSetNck(g_seq_handle[0]);
        SsVabClose(g_bgm_vab);
    }
    AdvSilenceBgm();
    AdvCloseBanks();
    while (g_cd_busy != -1) {
    }
    AdvRunFrame();
}
#else
INCLUDE_ASM("adv/nonmatchings/game/advmain", ovl_adv_entry);
#endif
