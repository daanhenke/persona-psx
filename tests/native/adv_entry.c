/* Run the production room-entry routine against synthetic RAM and platform adapters. */
#include <decomp/types.h>
/* Keep PS1 word fields 32-bit on LP64 hosts while pointers remain native. */
#define u_long u_int
#include <libetc.h>
#include "../../src/adv/game/advmain.c"
extern int    printf(const char*, ...);
extern void   abort(void);
extern void*  mmap(void*, unsigned long, int, int, int, long);
extern int    munmap(void*, unsigned long);
extern void*  memset(void*, int, unsigned long);
extern int    strcmp(const char*, const char*);
short         g_adv_enter_mode, g_adv_field_state, g_script_15C2, g_script_97C, g_bgm_ready, g_field_exit;
int           g_state_prev, g_state_next, g_29B0, g_29B4, g_exp_carry, g_image_queue_count, g_ot_index, g_pad_held[2], g_pad_pressed[2], g_BB94C, g_BB998;
u_short       g_map_id, g_script_534C, g_prev_map;
u_char        g_script_leave, g_adv_loading, g_adv_char_seq, g_actor_dim, g_cam_actor, g_fade_sprites, g_advcmd_loaded, g_options[8], g_pdata_index[256];
u_char        g_BC5B8, g_BC5BC, g_BC5C4, g_BC204, g_BC5C8;
GsOT          g_ot[2];
AdvScene*     g_adv_scene;
volatile int  g_cd_busy;
char          str_cd0_p_data[256 * 20], str_adv_ev01_bgd[] = "ev1", str_adv_ev02_bgd[] = "ev2", str_adv_ev03_bgd[] = "ev3";
typedef char  CheckCharSize[sizeof(Char) == 96 ? 1 : -1];
typedef char  CheckPersonaSize[sizeof(Persona) == 64 ? 1 : -1];
static int    cases, checks, frames, mode, leaveCode, mapCase, initialMap, newGame, sceneKind, flagValue, bgm;
static int    activeBuffer, clearCount, joins, expCalls, unpacks, rebuilds, loads, asyncLoads, scriptCalls, fieldTicks, fadeCalls, preload, volCalls, bgmCalls, slotCalls, closeCalls, soundResets, bankResets, cdTicks;
static int    scriptStarted, fieldStarted, enterCalls, bgmFade, leaveFlags;
static u_char script[4];
#define CHECK(x)                                                                                             \
    do                                                                                                       \
    {                                                                                                        \
        checks++;                                                                                            \
        if (!(x))                                                                                            \
        {                                                                                                    \
            printf("ADV entry failed line %d case %d mode %d leave %d\n", __LINE__, cases, mode, leaveCode); \
            abort();                                                                                         \
        }                                                                                                    \
    } while (0)
int VSync(int n)
{
    CHECK(n == 0);
    return frames;
}
void GsInitGraph(u_short x, u_short y, u_short m, u_short d, u_short v)
{
    CHECK(x == 320 && y == 240 && m == 256 && d == 0 && v == 0);
}
void func_80033A50(int a, int b, int c, int d)
{
    CHECK(a == 0 && b == 0 && c == 0 && d == 240);
}
void SetDispMask(int n)
{
    CHECK(n == 0);
}
void CharJoin(u_char c, u_char k, u_char l)
{
    CHECK(c == 0 && k == 1 && l == 5);
    joins++;
}
void CharSetLevelExp(u_char l, u_char s)
{
    CHECK(l == 5 && s == 0);
    expCalls++;
}
int ExpToLevel(u_char l, int u, int h)
{
    CHECK(l == 4 && u == 0 && h == 0);
    return 12345;
}
int GsGetActiveBuff(void)
{
    CHECK(g_ot[0].length == 11 && g_ot[1].length == 11);
    CHECK(g_ot[0].org == (GsOT_TAG*)0x800D6000);
    CHECK(g_ot[1].org == (GsOT_TAG*)0x800D9000);
    return activeBuffer;
}
void GsSetWorkBase(PACKET* p)
{
    CHECK(p == (PACKET*)(unsigned long)(0x800C0000 + activeBuffer * 0xB000));
}
void GsClearOt(u_short a, u_short b, GsOT* p)
{
    CHECK(a == 0 && b == 0 && clearCount < 2);
    CHECK(p == &g_ot[clearCount++]);
}
void VramClearRect(int x, int y, int w, int h)
{
    CHECK(clearCount == 2);
    CHECK(x == 0 && y == 0 && w == 320 && h == 479);
}
void LoadFileToAddr(const char* name, void* dest)
{
    CHECK(mode == 4 && dest == ROOM_DATA_AT);
    CHECK(name == (sceneKind == 2 ? str_adv_ev01_bgd : sceneKind == 3 ? str_adv_ev02_bgd
                                                                      : str_adv_ev03_bgd));
    loads++;
}
void LoadFileToAddrAsync(const char* name, void* dest)
{
    CHECK(dest == SCENE_AT);
    asyncLoads++;
    if (asyncLoads == 1)
    {
        CHECK(strcmp(name, "\\ADV\\ADVCMD.BIN;1") == 0);
        g_cd_busy = 0;
        cdTicks   = 2;
    }
    else
    {
        CHECK(g_adv_enter_mode == 4);
        CHECK(name == str_cd0_p_data + 3 * 20);
    }
}
void AdvRoomRebuild(void)
{
    CHECK(mode == 4);
    rebuilds++;
}
void AdvResetBanks(void)
{
    bankResets++;
}
void AdvCloseBanks(void)
{
    closeCalls++;
    g_cd_busy = -1;
}
void AdvSilenceBgm(void)
{
    CHECK(g_state_next == g_adv_enter_mode);
}
void SlotClearAll(void)
{
    CHECK(mode != 4);
}
void FadeBlackout(void)
{
    CHECK(mode != 4);
}
void ImageAnimStopAll(void)
{
    CHECK(mode != 4);
}
void AdvUnpack(u_char* src, u_char* dst, u_int size)
{
    CHECK(src == SCENE_PACKED && dst == (u_char*)SCENE_AT && size == (u_int)-1);
    unpacks++;
}
void bcopy(void* src, void* dst, int n)
{
    CHECK(src == SCENE_PACK_AT && dst == g_adv_pack && n == SCENE_PACK_LEN); /* Synthetic scene is prepared in place. */
}
void TimQueueAt(u_long* tim, short x, short y, short cx, short cy)
{
    CHECK(tim >= (u_long*)SCENE_AT && tim < (u_long*)((char*)SCENE_AT + 1024));
    CHECK(x >= 0 && y >= 0 && cx >= 0 && cy >= 0);
}
void FlushImageUploads(void) {}
int  DrawSync(int n)
{
    CHECK(n == 0);
    return 0;
}
void SsEnd(void)
{
    soundResets++;
}
void SsQuit(void)
{
    soundResets++;
}
void SsInit(void)
{
    soundResets++;
}
void SsSetMVol(short a, short b)
{
    CHECK(a == b);
    CHECK(a == (volCalls < 32 ? 127 - volCalls * 4 : 0));
    volCalls++;
}
void AdvLoadBgm(int n)
{
    CHECK(n == (flagValue ? 9 : 7));
    bgmCalls++;
}
int EventFlagGet(short id)
{
    CHECK(id == 4 || id == 12 || id == 300);
    return flagValue;
}
void AdvLoadEventBg(void)
{
    CHECK(mode != 4);
}
void AdvPickSceneByFlags(void)
{
    CHECK(mode != 4);
}
void ViewShakeStop(void) {}
void AdvRoomBgInit(void) {}
void MoonSetCells(void) {}
void PadLoadBindings(u_char c)
{
    CHECK(c == 3);
}
void PadSetPageButtons(u_char c)
{
    CHECK(c == 3);
}
void AdvFieldEnter(void)
{
    CHECK(g_adv_field_state == 1 && g_field_exit == 0);
    CHECK(g_pad_held[0] == 0 && g_pad_pressed[0] == 0);
    enterCalls++;
}
void TacticsSetCels(u_char n)
{
    CHECK(n == 2);
}
void AdvPickEffect(void) {}
void AdvEffectSetupSlots(void) {}
void FadeStepDown(u_char a, u_char b)
{
    CHECK(a == 0 && b == 0);
}
void FadeStepUp(u_char a, u_char b)
{
    CHECK(a == 2 && b == 128);
}
int FadeSpritesStep(short a, short b)
{
    CHECK(a == 2 && b == 128);
    return ++fadeCalls >= 3;
}
void AdvRunFrame(void)
{
    CHECK(++frames < 200);
    if (cdTicks && --cdTicks == 0)
        g_cd_busy = -1;
}
void SlotSetPos(u_char slot, int attr, short x, short y)
{
    int pair    = slotCalls / 2;
    int expectY = pair < 18 ? -20 + pair * 2 : 14 - (pair - 18) * 2;
    CHECK(slot == (slotCalls % 2 ? BAR_SLOT2 : BAR_SLOT));
    CHECK(attr == (slotCalls % 2 ? 0x34 : 0x35));
    CHECK(x == (slotCalls % 2 ? 227 : 224));
    CHECK(y == expectY);
    slotCalls++;
}
int AdvRunScript(u_char* p)
{
    CHECK(p == script);
    CHECK(!scriptStarted);
    scriptStarted = 1;
    scriptCalls++;
    return leaveCode;
}
void AdvFadeUpBlocking(short a, short b)
{
    CHECK(a == 8 && b == 128);
}
void MapRevealScene(void)
{
    fieldStarted = 1;
}
void AdvFieldTick(void)
{
    CHECK(fieldStarted);
    fieldTicks++;
    g_field_exit = 255;
}
void AdvLeaveMapFlag(void)
{
    leaveFlags++;
}
void AdvFadeBgmOut(void)
{
    bgmFade++;
}
void PreloadDng(void)
{
    preload = 1;
}
void PreloadS2d(void)
{
    preload = 2;
}
void PreloadBtlField(void)
{
    preload = 3;
}
void PreloadName(void)
{
    preload = 4;
}
void PreloadAdv(void)
{
    preload = 5;
}
void AdvFadeDownBlocking(short a, short b)
{
    CHECK(a == 2 && b == 0);
}
void SsSetNck(short n)
{
    CHECK(0);
}
void SsVabClose(short n)
{
    CHECK(0);
}
static void one(void)
{
    int i, expected, expectedPreload, expectedFade;
    cases++;
    memset((void*)0x800C0000, 0, 0x140000);
    memset(g_ot, 0, sizeof g_ot);
    memset(g_options, 0, sizeof g_options);
    frames = clearCount = joins = expCalls = unpacks = rebuilds = loads = asyncLoads = scriptCalls = fieldTicks = fadeCalls = preload = volCalls = bgmCalls = slotCalls = closeCalls = soundResets = bankResets = cdTicks = scriptStarted = fieldStarted = enterCalls = bgmFade = leaveFlags = 0;
    g_state_prev                                                                                                                                                                                                                                                                             = mode;
    g_map_id                                                                                                                                                                                                                                                                                 = initialMap;
    g_script_534C                                                                                                                                                                                                                                                                            = 11;
    g_script_15C2                                                                                                                                                                                                                                                                            = -1;
    g_field_exit                                                                                                                                                                                                                                                                             = 99;
    g_cd_busy                                                                                                                                                                                                                                                                                = -1;
    g_exp_carry                                                                                                                                                                                                                                                                              = -123;
    g_29B0 = g_29B4 = 0;
    g_BC5B8 = g_BC5BC = g_BC5C4 = g_BC204 = g_BC5C8 = g_actor_dim = g_adv_loading = g_adv_char_seq = 9;
    g_BB94C = g_BB998 = g_image_queue_count = 9;
    g_cfg[2]                                = 2;
    g_pad_config                            = 3;
    g_adv_room                              = 8;
    g_script_resuming                       = 0;
    g_pdata_index[11]                       = 3;
    MAP_ID_AT                               = initialMap;
    GOTO_MAP_AT                             = 13;
    for (i = 0; i < 4; i++)
        SCENE_AT[i] = 128 + i * 64;
    g_adv_scene          = (AdvScene*)0x80100034;
    g_adv_scene->kind    = sceneKind;
    g_adv_scene->arrive  = script;
    g_adv_pack->bgm.flag = 12;
    g_adv_pack->bgm.id   = bgm ? 7 : 65535;
    g_adv_pack->bgm.alt  = bgm ? 9 : 65535;
    for (i = 0; i < 5; i++)
        g_chars[i].unk5D = 7;
    g_chars[0].unk10 = 4321;
    ovl_adv_entry();
    expected = mode;
    if (leaveCode == 1)
        expected = initialMap == 0 ? 6 : initialMap == 463 ? -1
                                                           : 3;
    if (leaveCode == 2)
        expected = 1;
    if (leaveCode == 3)
        expected = 2;
    if (leaveCode == 4)
        expected = 0;
    if (leaveCode == 5)
        expected = 4;
    if (leaveCode == 6)
        expected = 6;
    if (leaveCode == 7)
        expected = -1;
    CHECK(g_state_next == expected && g_adv_enter_mode == expected);
    CHECK(clearCount == 2 && enterCalls == 1 && scriptCalls == 1 && bankResets == 1 && closeCalls == 1);
    CHECK(joins == newGame && expCalls == newGame);
    if (newGame)
    {
        CHECK(g_chars[0].unk1C == 12345);
        CHECK(g_29B0 == 1000000 && g_29B4 == 300000 && g_options[0] == 1);
        for (i = 0; i < 225; i++)
            CHECK(g_formation_preset[i] == 255);
        for (i = 0; i < 16; i++)
            CHECK(g_persona_slots[i] == 255);
        for (i = 0; i < 31; i++)
        {
            CHECK(g_personas[i].key == 0);
            CHECK(g_personas[i].owner == 255);
        }
    }
    CHECK(g_adv_room == 7);
    for (i = 0; i < 5; i++)
        CHECK(g_chars[i].unk5D == 0);
    CHECK(g_prev_map == initialMap && g_script_97C == 256 && g_advcmd_loaded == 1 && g_script_resuming == 0);
    CHECK(g_actor_dim == 0 && g_adv_loading == 0 && g_adv_char_seq == 0 && g_BC5B8 == 0 && g_BC5BC == 0 && g_BC5C4 == 0 && g_BC204 == 0 && g_BC5C8 == 0 && g_BB94C == 0 && g_BB998 == 0 && g_image_queue_count == 0);
    CHECK(unpacks == (mode != 4) && rebuilds == (mode == 4));
    CHECK(loads == (mode == 4 && sceneKind >= 2 && sceneKind <= 4));
    CHECK(soundResets == (mode == 1 ? 3 : 0));
    CHECK(bgmCalls == (mode != 4 && bgm));
    CHECK(volCalls == (bgmCalls ? 33 : 0));
    CHECK(fieldTicks == (leaveCode == 0));
    CHECK(slotCalls == 78);
    expectedPreload = expected == 0 ? 1 : expected == 2 ? 2
                                      : expected == 1   ? 3
                                      : expected == 5   ? 4
                                      : expected == 3   ? 5
                                                        : 0;
    expectedFade    = expected == 0 || expected == 1 || expected == 2 || expected == 5 || expected == 6;
    CHECK(preload == expectedPreload && bgmFade == expectedFade);
    CHECK(leaveFlags == (expectedPreload != 0 && expected != 5));
    CHECK(asyncLoads == (expected == 4 ? 2 : 1));
    if (expected == 4)
        CHECK(g_exp_carry == (flagValue ? -123 : 4321));
    if (leaveCode == 2)
        CHECK(g_map_id == 11);
    if (leaveCode == 1 && initialMap == 0)
        CHECK(g_script_15C2 == 17);
    if (leaveCode == 1 && initialMap != 0 && initialMap != 463)
        CHECK(MAP_ID_AT == 13);
    CHECK(fadeCalls == (initialMap == 3 && !flagValue ? 0 : 3));
    CHECK(g_fade_sprites == 0 && g_cd_busy == -1);
}
int main(void)
{
    void* ram = mmap((void*)0x800C0000, 0x140000, 3, 0x100022, -1, 0);
    CHECK(ram == (void*)0x800C0000);
    for (mode = -1; mode <= 6; mode++)
        for (leaveCode = 0; leaveCode <= 7; leaveCode++)
            for (mapCase = 0; mapCase < 4; mapCase++)
                for (flagValue = 0; flagValue < 2; flagValue++)
                {
                    initialMap   = mapCase == 0 ? 0 : mapCase == 1 ? 2
                                                  : mapCase == 2   ? 463
                                                                   : 3;
                    newGame      = initialMap == 0;
                    sceneKind    = cases % 5;
                    activeBuffer = cases & 1;
                    bgm          = (cases / 2) & 1;
                    one();
                }
    CHECK(munmap(ram, 0x140000) == 0);
    printf("ADV entry: %d cases / %d checks passed\n", cases, checks);
    return 0;
}
