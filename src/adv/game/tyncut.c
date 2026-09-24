/* Persona 1 (JP) - the TYN event cutscene.  ADV only.
 *   0x80088C70 AdvTynCutscene
 *
 * A fully scripted scene, played in place of the usual event cutscene when
 * the scene asks for it (script opcode 0x2A with g_cutscene_alt set). It
 * owns the screen from start to finish and puts the room back afterwards.
 *
 *  - Its VAB and sequences come out of the unpacked scene file at
 *    0x80118000 (entries 1 and 10 are the VAB, 0 and 2..9 the sequences);
 *    TYNCHR.BIN's images go to VRAM, and three pictures named by
 *    g_event_args are read through AdvResolveSceneLoc.
 *  - The scene is drawn in 3D (GsInit3D; func_8008A93C draws, spun by
 *    g_char_rot) with sprites from the scene pack's table at 0x801000A0,
 *    grown, faded and slid by hand one frame at a time. A burst of sparks is
 *    thrown with rsin/rcos and rand, and two message windows play over a
 *    box, the second with a character's name poked into it.
 *  - At the end the music fades out, track 0x3A starts, ADVCHR.BIN is read
 *    and unpacked, and the room is rebuilt the way ovl_adv_entry builds it,
 *    down to AdvScriptSpecial(0x85) bringing the command bar back.
 *
 * func_8008A93C is handed a local the original never sets.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <libcd.h>
#include <persona/adv/scene.h>
#include <persona/common/bg.h>
#include <persona/common/slot.h>
#include <persona/common/imageanim.h>

#define SCENE_AT   ((u_long *)0x80118000)
#define TYN_AT     ((u_long *)0x80120000)
#define PICTURE_AT ((u_long *)0x800F4000)
#define MEMBER(at, n) ((u_long *)((u_char *)(at) + (at)[n]))

#define g_slots      ((Slot *)0x800DC10C)
#define g_seq_handle ((short *)0x801F537C)
#define g_vab_id     ((short *)0x801F535C)
#define g_bgm_vab    (*(short *)0x801F5360)

/* The scene pack's sprite table, by byte offset into the pack. */
#define PACK_SPRITE(off) (*(void **)(0x80100000 + (off)))

/* Five arguments the scene sets up for the cutscene: the face and two
   pictures to read, whether to show the extra sprite, whether to talk. */
extern u_short g_event_args[5];

extern SVECTOR g_char_rot[2];
extern SVECTOR g_quad_verts[4];
extern u_char  g_tyn_msg1[];
extern u_char  g_tyn_msg2[];
extern u_char  g_tyn_face_def[];
extern u_char  g_tyn_glow_def[];
extern u_short g_tyn_glow_speed;
extern u_char  g_bar_def[];
extern u_char  g_bar_def2[];
extern u_char  g_adv_char_seq;
extern int     g_image_queue_count;
extern int     g_ot_index;
extern GsOT    g_ot[];
extern int     g_effect_tick;
extern int     g_pad_held[];
extern Slot   *g_slot_cur;
extern AdvScene *g_adv_scene;
extern CdlFILE g_adv_scene_file;
extern volatile int g_cd_busy;

extern void LoadFileToAddr(const char *name, void *dest);
extern void LoadFileToAddrAsync(const char *name, void *dest);
extern void CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *dest);
extern void AdvResolveSceneLoc(short kind, int index, void *unused);
extern void AdvRenderCharFrame(short extra);
extern void AdvRunFrame(void);
extern void AdvFadeUpBlocking(short step, short limit);
extern void AdvFadeDownBlocking(short step, short floor);
extern void AdvBoxFadeUp(short step, short from, short to, short x, u_short y,
                         short char_frame);
extern void AdvBoxFadeDown(short step, short from, short to, short x,
                           u_short y, short char_frame);
extern void AdvUnpack(u_char *src, u_char *dst, u_int size);
extern void AdvLoadBgm();
extern void AdvLoadEventBg(void);
extern void AdvPickSceneByFlags(void);
extern void AdvPickEffect(void);
extern void AdvEffectSetupSlots(void);
extern void AdvSceneStartImages(void);
extern void AdvScriptSpecial(u_char n);
extern void TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void SlotClearAll(void);
extern void FadeBlackout(void);
extern void SlotSetSemiTrans(u_char slot, u_char on);
extern void SlotFadeIn(u_char slot, u_char step);
extern void CamCenterOnActor(u_char actor);
extern void ActorsSetDepth();
extern void ActorsPlaceSprites(void);
extern void SlotsApplyXScale(void);
extern int  MsgStep(void);
extern void func_80034850(u_long *base);
extern void func_8008AE80(void);
extern void func_8008AE2C(void);
extern void func_8008A9B4(void);
extern void func_8008AC50(void);
extern void func_8008A93C(int obj, int pad);
extern void func_800831D4(u_char kind);
extern void func_80084694(void);

/* Grows a slot's scale a step towards 1.0, faster in y than in x. */
#define GROW(s)                                                                \
    if ((s)->scale_y < 0x1000) {                                               \
        (s)->scale_y += 0x200;                                                 \
    } else {                                                                   \
        (s)->scale_y = 0x1000;                                                 \
    }                                                                          \
    if ((s)->scale_x < 0x1000) {                                               \
        (s)->scale_x += 0x88;                                                  \
    } else {                                                                   \
        (s)->scale_x = 0x1000;                                                 \
    }

/* Opens sequence n of the scene file on the event VAB and plays it. */
#define PLAY(slot, n)                                                          \
    g_seq_handle[slot] = SsSeqOpen(MEMBER(SCENE_AT, n), g_bgm_vab);            \
    SsSeqSetVol(g_seq_handle[slot], 0x7F, 0x7F);                               \
    SsSeqPlay(g_seq_handle[slot], 1, 1)

#define WAIT_CD()                                                              \
    while (g_cd_busy != -1) {                                                  \
    }

#ifdef NON_MATCHING
void AdvTynCutscene(void)
{
    GsBOXF box = { 0x40000000, -0x7C, 0x28, 0xF8, 0x38, 0x20, 0x20, 0x20 };
    int    i;
    int    k;
    short  x;
    short  y;
    int    n;
    int    vol;
    int    top;
    int    steps;
    int    obj;

    g_adv_char_seq = 1;
    WAIT_CD();
    g_bgm_vab = SsVabOpenHead((u_char *)MEMBER(SCENE_AT, 1), -1);
    SsVabTransBody((u_char *)MEMBER(SCENE_AT, 10), g_bgm_vab);
    while (!SsVabTransCompleted(SS_IMEDIATE)) {
    }
    SsSeqSetVol(g_seq_handle[0], 0, 0);
    SlotClearAll();
    FadeBlackout();
    ImageAnimStopAll();
    AdvRunFrame();

    LoadFileToAddrAsync("\\ADV\\TYNCHR.BIN;1", TYN_AT);
    WAIT_CD();
    SsSetNck(g_seq_handle[0]);
    g_seq_handle[0] = SsSeqOpen(MEMBER(SCENE_AT, 0), g_vab_id[0]);
    SsSeqSetVol(g_seq_handle[0], 0x7F, 0x7F);
    SsSeqPlay(g_seq_handle[0], 1, g_vab_id[0]);

    g_image_queue_count = 0;
    TimQueueAt(MEMBER(TYN_AT, 0), 0x180, 0, 0, 0x1E0);
    TimQueueAt(MEMBER(TYN_AT, 1), 0x280, 0, 0, 0x1E2);
    TimQueueAt(MEMBER(TYN_AT, 2), 0x300, 0, 0, 0x1E3);
    TimQueueAt(MEMBER(TYN_AT, 3), 0x340, 0x100, 0, 0x1E4);
    TimQueueAt(MEMBER(TYN_AT, 4), 0x280, 0x100, 0, 0x1E5);
    TimQueueAt(MEMBER(TYN_AT, 5), 0x300, 0x100, 0, 0x1E7);
    TimQueueAt(MEMBER(TYN_AT, 6), 0x240, 0, 0x100, 0x1E0);
    FlushImageUploads();
    DrawSync(0);

    AdvResolveSceneLoc(5, g_event_args[0], 0);
    CdReadFileToAddrAsync(&g_adv_scene_file, 8, PICTURE_AT);
    WAIT_CD();
    TimQueueAt(PICTURE_AT + 2, 0x140, 0x100, 0, 0x1E6);
    AdvRunFrame();
    AdvResolveSceneLoc(4, g_event_args[1], 0);
    CdReadFileToAddrAsync(&g_adv_scene_file, 9, PICTURE_AT);
    WAIT_CD();
    TimQueueAt(PICTURE_AT + 2, 0x200, 0x100, 0, 0x1F2);
    AdvRunFrame();
    AdvResolveSceneLoc(4, g_event_args[2], 0);
    CdReadFileToAddrAsync(&g_adv_scene_file, 9, PICTURE_AT);
    WAIT_CD();
    TimQueueAt(PICTURE_AT + 2, 0x200, 0x180, 0, 0x1F3);
    AdvRunFrame();

    GsInit3D();
    g_ot[1].length = 11;
    g_ot[0].length = 11;
    g_ot[0].org = (GsOT_TAG *)0x800D6000;
    g_ot[1].org = (GsOT_TAG *)0x800D9000;
    g_ot_index = GsGetActiveBuff();
    func_80034850((u_long *)(0x800C0000 + g_ot_index * 0xB000));
    GsClearOt(0, 0, &g_ot[g_ot_index]);
    g_bg_shown = 0;
    SlotClearAll();
    FadeBlackout();
    ImageAnimStopAll();
    func_8008AE80();
    func_8008AE2C();
    func_8008A9B4();
    func_8008AC50();
    LoadFileToAddrAsync("\\ADV\\TYN01.BIN;1", TYN_AT);

    SlotInitTagged(g_tyn_face_def, 2, 0x3FE, -0xA0, -0x78);
    SlotInit(PACK_SPRITE(0xEC), 0, 0x3FF, -0xA0, -0x78);
    SlotInit(PACK_SPRITE(0xD0), 0x40, 8, -0x61, 0x5B);
    SlotInit(PACK_SPRITE(0xC4), 0x41, 8, -0x3A, 0x47);
    SlotInit(PACK_SPRITE(0xA0), 1, 0x300, -0x64, 2);
    AdvFadeUpBlocking(8, 0x80);
    SlotInit(PACK_SPRITE(0xD0), 0x40, 8, -0x61, 0x5B);
    SlotInit(PACK_SPRITE(0xC4), 0x41, 8, -0x3A, 0x47);
    for (i = 0; i < 0x40; i++) {
        AdvRenderCharFrame(0);
    }
    PLAY(1, 2);
    SlotInit(PACK_SPRITE(0xC8), 0x41, 8, -0x3A, 0x47);
    for (i = 0; i < 0x40; i++) {
        AdvRenderCharFrame(0);
    }
    SlotInit(PACK_SPRITE(0xC4), 0x41, 8, -0x3A, 0x47);
    for (i = 0; i < 0x40; i++) {
        AdvRenderCharFrame(0);
    }
    while (g_cd_busy != -1) {
        AdvRenderCharFrame(0);
    }
    TimQueueAt(MEMBER(TYN_AT, 0), 0x180, 0x100, 0, 0x1F0);
    AdvRenderCharFrame(0);

    g_tyn_glow_speed = 0x1100;
    if (g_event_args[3]) {
        SlotInit(PACK_SPRITE(0xE8), 0x18, 0x108, 0x2B, 0x15);
        for (i = 0; i < 10; i++) {
            AdvRenderCharFrame(0);
        }
    }

    /* The first glow grows out. */
    PLAY(2, 3);
    SlotInit(PACK_SPRITE(0xB8), 0x12, 0x100, 0x10, -2);
    SlotSetSemiTrans(0x12, 1);
    for (i = 0; i < 0x24; i++) {
        AdvRenderCharFrame(0);
    }
    SlotInitTagged(g_tyn_glow_def, 0x10, 0x110, 0x50, -2);
    SlotSetSemiTrans(0x10, 1);
    SlotSetBrightness(0x10, 0);
    SlotFadeIn(0x10, 2);
    g_slot_cur = &g_slots[0x10];
    g_slots[0x10].mx = 0x40;
    g_slots[0x10].my = 0x50;
    g_slots[0x10].scale_y = 0x2AA;
    g_slots[0x10].scale_x = 0x55;
    for (i = 0; i < 0x20; i++) {
        g_slot_cur = &g_slots[0x10];
        GROW(g_slot_cur);
        AdvRenderCharFrame(0);
    }

    /* And the second beside it. */
    PLAY(3, 3);
    SlotInit(PACK_SPRITE(0xB8), 0x13, 0x100, 0x3E, 0x10);
    SlotSetSemiTrans(0x13, 1);
    for (i = 0; i < 0x24; i++) {
        AdvRenderCharFrame(0);
    }
    SlotInitTagged(g_tyn_glow_def, 0x11, 0x100, 0x7E, 0x10);
    SlotSetAnim(0x11, 0, 0, 0, 0, 0x80, 0, 1);
    SlotSetSemiTrans(0x11, 1);
    SlotSetBrightness(0x11, 0);
    SlotFadeIn(0x11, 2);
    g_slot_cur = &g_slots[0x11];
    g_slots[0x11].mx = 0x40;
    g_slots[0x11].my = 0x50;
    g_slots[0x11].scale_y = 0x2AA;
    g_slots[0x11].scale_x = 0x55;
    for (i = 0; i < 0x40; i++) {
        g_slot_cur = &g_slots[0x10];
        GROW(g_slot_cur);
        g_slot_cur++;
        GROW(g_slot_cur);
        AdvRenderCharFrame(0);
    }

    /* Something rises into view. */
    TimQueueAt(MEMBER(TYN_AT, 2), 0x180, 0x100, 0, 0x1F0);
    AdvRenderCharFrame(0);
    PLAY(4, 4);
    SlotInit(PACK_SPRITE(0xB0), 0x12, 2, 0, 0);
    top = 10;
    steps = 0x60;
    for (i = 0; i < 0x60; i++) {
        SlotSetPos(0x12, 0x80, 0x28, top - (steps - i) * 2);
        AdvRenderCharFrame(0);
    }
    AdvRenderCharFrame(0);
    PLAY(5, 5);
    PLAY(6, 6);
    SlotClear(0x10);
    SlotClear(0x11);
    SlotClear(0x18);
    for (i = 0; i < 10; i++) {
        AdvRenderCharFrame(0);
    }

    /* A flash. */
    SlotInit(PACK_SPRITE(0xB4), 0x12, 0x80, 0x28, 10);
    SlotInit(PACK_SPRITE(0xA8), 1, 0x300, -0x64, 2);
    AdvBoxFadeUp(0x20, 0x80, 0xFF, -0xA0, -0x78, 1);
    g_tyn_glow_speed = 0x2100;
    SlotInitTagged(g_tyn_glow_def, 0x10, 1, 0x50, -2);
    SlotInitTagged(g_tyn_glow_def, 0x11, 1, 0x7E, 0x10);
    SlotInit(PACK_SPRITE(0xE8), 0x18, 0x108, 0x2B, 0x15);
    SlotSetBrightness(0x18, 0xFF);
    SlotSetAnim(0x11, 0, 0, 0, 0, 0x80, 0, 1);
    SlotSetBrightness(0x10, 0xFF);
    SlotSetBrightness(0x11, 0xFF);
    SlotSetSemiTrans(0x10, 1);
    SlotSetSemiTrans(0x11, 1);
    SlotSetSemiTrans(0x40, 1);
    SlotSetSemiTrans(0x41, 1);
    g_slot_cur = &g_slots[0x10];
    g_slots[0x10].mx = 0x40;
    g_slots[0x10].my = 0x50;
    g_slot_cur = &g_slots[0x11];
    g_slots[0x11].mx = 0x40;
    g_slots[0x11].my = 0x50;
    AdvBoxFadeUp(0x20, 0x80, 0xFF, -0xA0, -0x78, 1);
    SlotClear(0x10);
    SlotClear(0x11);
    SlotClear(0x18);
    SlotSetSemiTrans(0x40, 0);
    SlotSetSemiTrans(0x41, 0);
    SlotInit(PACK_SPRITE(0xD4), 0x40, 8, -0x61, 0x5B);
    SlotInit(PACK_SPRITE(0xCC), 0x41, 8, -0x3A, 0x47);
    for (i = 0; i < 4; i++) {
        AdvRenderCharFrame(0);
    }
    SlotInit(PACK_SPRITE(0xB4), 0x13, 0x78, 0x28, 10);
    SlotSetSemiTrans(0x13, 1);
    SlotSetBrightness(0x13, 0x60);
    for (i = 0; i < 4; i++) {
        AdvRenderCharFrame(0);
    }
    SlotInit(PACK_SPRITE(0xB4), 0x14, 0x78, 0x28, 10);
    SlotSetSemiTrans(0x14, 1);
    SlotSetBrightness(0x14, 0x40);

    /* Sparks, two every third frame, thrown round in circles. */
    for (i = 0; i < 0x60; i++) {
        if (i % 3 == 0) {
            k = i / 3;
            x = rsin(g_effect_tick * 108) / 50 + ((rand() & 0xF) + 8);
            y = rcos(g_effect_tick * 108) / 50 + ((rand() & 0xF) + 0x18);
            SlotInit(PACK_SPRITE(0xC0), (k & 7) | 0x20, 1, x, y);
            SlotSetSemiTrans((k & 7) | 0x20, 1);
            x = rsin(g_effect_tick * 208) / 30 + ((rand() & 0xF) + 8);
            y = rcos(g_effect_tick * 208) / 30 + ((rand() & 0xF) + 0x18);
            SlotInit(PACK_SPRITE(0xC0), (k & 7) | 0x28, 1, x, y);
            SlotSetSemiTrans((k & 7) | 0x28, 1);
        }
        AdvRenderCharFrame(0);
    }
    SlotInit(PACK_SPRITE(0xA0), 1, 0x300, -0x64, 2);
    SlotInit(PACK_SPRITE(0xB0), 0x12, 0x80, 0x28, 10);
    SlotClear(0x13);
    SlotClear(0x14);
    for (i = 0; i < 0x10; i++) {
        SlotClear(i + 0x20);
    }
    SlotInit(PACK_SPRITE(0xD8), 0x40, 8, -0x61, 0x5B);
    SlotInit(PACK_SPRITE(0xC4), 0x41, 8, -0x3A, 0x47);
    PLAY(7, 7);
    AdvBoxFadeUp(0x20, 0x80, 0xFF, -0xA0, -0x78, 1);

    /* The first message, over a box. */
    if (g_event_args[4]) {
        AdvBoxFadeUp(0x20, 0, 0xFF, -0xA0, -0x78, 1);
        AdvBoxFadeUp(0x20, 0, 0xFF, -0xA0, -0x78, 1);
        AdvBoxFadeUp(0x20, 0, 0xFF, -0xA0, -0x78, 1);
        BgMapInit(g_tyn_msg1, 0);
        i = (short)g_bg_layer_otz[4];
        g_bg_layers[4].x = -0x78;
        g_bg_layers[4].y = 0x2C;
        g_bg_layers[4].w = 0xF0;
        g_bg_layers[4].h = 0x30;
        g_bg_layer_otz[4] = 1;
        g_bg_shown |= 0x10;
        while (!(g_msg->flags & MSG_DONE)) {
            GsSortBoxFill(&box, &g_ot[g_ot_index], 2);
            MsgStep();
            AdvRenderCharFrame(0);
        }
        g_bg_layer_otz[4] = i;
        g_bg_shown ^= 0x10;
    } else {
        AdvBoxFadeDown(2, 0xFF, 0x80, -0xA0, -0x78, 1);
    }

    /* The figure turns, while rings rise. */
    TimQueueAt(MEMBER(TYN_AT, 1), 0x180, 0x100, 0, 0x1F0);
    AdvRenderCharFrame(0);
    PLAY(8, 8);
    for (i = 0; i < 0x55; i++) {
        if (i % 60 == 0) {
            k = (i / 60 & 0xF) | 0x30;
            SlotInit(PACK_SPRITE(0xBC), k, 0x80, 0x1E, 0x1B);
            SlotSetSemiTrans(k, 1);
        }
        g_char_rot[1].vy += 0x80;
        g_char_rot[0].vy += 0x80;
        AdvRenderCharFrame(1);
        func_8008A93C(obj, g_pad_held[0]);
        g_slot_cur = &g_slots[0x12];
        g_slots[0x12].y -= i / 16;
    }
    SlotClear(0x12);

    /* The second message names a character. */
    g_tyn_msg2[0xE] = g_tyn_msg2[0x13] = g_event_args[0];
    BgMapInit(g_tyn_msg2, 0);
    n = 0x60;
    i = (short)g_bg_layer_otz[4];
    g_bg_layers[4].x = -0x78;
    g_bg_layers[4].y = 0x2C;
    g_bg_layers[4].w = 0xF0;
    g_bg_layers[4].h = 0x30;
    g_bg_layer_otz[4] = 1;
    g_bg_shown |= 0x10;
    while (!(g_msg->flags & MSG_DONE)) {
        GsSortBoxFill(&box, &g_ot[g_ot_index], 2);
        MsgStep();
        if (n % 60 == 0) {
            k = (n / 60 & 0xF) | 0x30;
            SlotInit(PACK_SPRITE(0xBC), k, 0x80, 0x1E, 0x1B);
            SlotSetSemiTrans(k, 1);
        }
        g_char_rot[1].vy += 0x80;
        g_char_rot[0].vy += 0x80;
        func_8008A93C(obj, g_pad_held[0]);
        AdvRenderCharFrame(1);
        n++;
    }
    g_bg_layer_otz[4] = i;
    g_bg_shown ^= 0x10;

    /* The quad opens out as the figure spins faster. */
    PLAY(9, 9);
    for (i = 0; i < 0x40; i++) {
        g_char_rot[1].vy += 0xA0;
        g_char_rot[0].vy += 0xA0;
        g_quad_verts[0].vx -= 3;
        g_quad_verts[0].vy -= 3;
        g_quad_verts[1].vx += 3;
        g_quad_verts[1].vy -= 3;
        g_quad_verts[2].vx -= 3;
        g_quad_verts[2].vy += 3;
        g_quad_verts[3].vx += 3;
        g_quad_verts[3].vy += 3;
        func_8008A93C(obj, g_pad_held[0]);
        AdvRenderCharFrame(1);
    }
    AdvFadeDownBlocking(8, 0);
    g_adv_char_seq = 0;

    /* Back to the room. */
    for (i = 0, vol = 0x7F; i < 0x20; i++, vol -= 4) {
        SsSetMVol(vol, vol);
        VSync(0);
    }
    SsSetMVol(0, 0);
    AdvLoadBgm(0x3A);
    GsInitGraph(0x140, 0xF0, 0x100, 0, 0);
    g_bg_shown = 1;
    LoadFileToAddr("\\ADV\\ADVCHR.BIN;1", (void *)0x80180000);
    AdvUnpack((u_char *)0x80180000, (u_char *)SCENE_AT, -1);
    TimQueueAt(MEMBER(SCENE_AT, 3), 0x394, 0x60, 0, 0x1E4);
    TimQueueAt(MEMBER(SCENE_AT, 2), 0x388, 8, 0, 0x1E4);
    TimQueueAt(MEMBER(SCENE_AT, 1), 0x380, 0, 0, 0x1E2);
    TimQueueAt(MEMBER(SCENE_AT, 0), 0x140, 0, 0x110, 0x1E0);
    FlushImageUploads();
    DrawSync(0);
    AdvLoadEventBg();
    AdvPickSceneByFlags();
    FlushImageUploads();
    AdvPickEffect();
    AdvEffectSetupSlots();
    func_800831D4(g_adv_scene->kind);
    SlotClearAll();
    SlotInitTagged(g_bar_def, 0x29, 0x35, 0xE0, -0x20);
    SlotInit(g_bar_def2, 0x2A, 0x34, 0xE3, -0x20);
    AdvSceneStartImages();
    func_80084694();
    CamCenterOnActor(0);
    ActorsSetDepth(0, 0xF);
    ActorsPlaceSprites();
    SlotsApplyXScale();
    AdvScriptSpecial(0x85);
}
#else
INCLUDE_ASM("adv/nonmatchings/game/tyncut", AdvTynCutscene);
#endif
