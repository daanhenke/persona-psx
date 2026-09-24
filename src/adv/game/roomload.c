/* Persona 1 (JP) - putting a room back up after its files are loaded.  ADV only.
 *   0x80085958 AdvSceneFadeOut   0x80085A60 AdvQueueCmdBar
 *   0x80085AE0 AdvRoomRebuild    0x80085CA8 AdvQueueShadows
 *
 * The ADV loaders (eventfiles.c) read the scene's files; these put what they
 * read on screen. ADVCMD.BIN unpacks to 0x80118000 and its first two members
 * are the command bar's images; the KAGE entry's eight shadow images hang off
 * the shadow sprite's definition. Rebuilding a room redoes everything the
 * room's scene pack describes - actors, effects, the bar, the shadows, the
 * camera and the scene's own images - without reading anything again.
 */
#include <decomp/types.h>
#include <persona/adv/actor.h>
#include <persona/adv/scene.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)0x800DC10C)

/* ADVCMD.BIN, unpacked, and its members. */
#define ADVCMD_AT       ((u_long *)0x80118000)
#define ADVCMD_MEMBER(n) ((u_long *)((u_char *)ADVCMD_AT + ADVCMD_AT[n]))

/* The shadow sprite's definition carries its eight images from +0x18. */
#define KAGE_TIM(n) (((u_long **)g_kage_sprite)[6 + (n)])

#define BAR_SLOT  0x29
#define BAR_SLOT2 0x2A

extern Slot   *g_slot_cur;
extern u_long  g_bg_shown;
extern u_char  g_kage_sprite[];
extern u_char  g_bar_def[];
extern u_char  g_bar_def2[];

extern void SlotClearAll(void);
extern void SlotInit(void *def, u_char slot, int attr, short x, short y);
extern void FadeBlackout(void);
extern void ImageAnimStopAll(void);
extern void VramClearRect(int x, int y, int w, int h);
extern void TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void AdvFadeDownBlocking(short step, short floor);
extern void AdvEffectSetupSlots(void);
extern void AdvSceneStartImages(void);
extern void ActorsPlaceSprites(void);
extern void CamCenterOnActor(u_char actor);
extern void func_800831D4(u_char kind);
extern void func_800837E8(void);
extern void func_80084694(void);
extern void func_80088B8C(void);

void AdvQueueShadows(void);

/* ActorsSetDepth (actors.c), expanded here as the original did. */
static inline void RoomSetDepth(u_short actor)
{
    AdvActor *a;
    int       y;

    y = g_adv_actors[actor].y;
    for (a = g_adv_actors; (long)a < (long)&g_adv_actors[ACTOR_COUNT]; a++) {
        if (a->id == ACTOR_NONE) {
            a->depth = 0;
        } else if (y >= a->y) {
            a->depth = DEPTH_BEHIND;
        } else {
            a->depth = 0;
        }
    }
}

/* SlotsApplyXScale (slotxscale.c), likewise. */
static inline void RoomApplyXScale(void)
{
    u_char i;

    for (i = 0; i < 8; i++) {
        g_slot_cur = &g_slots[i];
        if (g_slots[i].attr & SLOT_ATTR_XSCALE) {
            g_slots[i].scale_x = 0xFFF;
        } else {
            g_slots[i].scale_x = 0x1000;
        }
    }
}

/* Puts the actors' sprites back in order and fades the room out. */
void AdvSceneFadeOut(void)
{
    RoomSetDepth(0);
    ActorsPlaceSprites();
    RoomApplyXScale();
    AdvFadeDownBlocking(8, 0);
}

/* The command bar's two images, out of ADVCMD.BIN. */
void AdvQueueCmdBar(void)
{
    VramClearRect(0, 0, 0x140, 0x1DF);
    TimQueueAt(ADVCMD_MEMBER(1), 0x380, 0x1C8, 0x100, 0x1F8);
    TimQueueAt(ADVCMD_MEMBER(0), 0x380, 0x100, 0x3C0, 0x1A0);
}

void AdvRoomRebuild(void)
{
    SlotClearAll();
    FadeBlackout();
    ImageAnimStopAll();
    func_800837E8();
    func_80088B8C();
    g_bg_shown = 1;
    func_800831D4(g_adv_scene->kind);
    AdvEffectSetupSlots();
    SlotInitTagged(g_bar_def, BAR_SLOT, 0x35, 0xE0, 0xC);
    SlotInit(g_bar_def2, BAR_SLOT2, 0x34, 0xE3, 0xC);
    AdvQueueShadows();
    CamCenterOnActor(0);
    AdvSceneStartImages();
    func_80084694();
    RoomSetDepth(g_cam_actor);
    ActorsPlaceSprites();
    RoomApplyXScale();
}

/* The KAGE entry's eight shadow images, one VRAM column each. */
void AdvQueueShadows(void)
{
    TimQueueAt(KAGE_TIM(0), 0x180, 0x100, 0, 0x1F0);
    TimQueueAt(KAGE_TIM(1), 0x1C0, 0x100, 0, 0x1F1);
    TimQueueAt(KAGE_TIM(2), 0x200, 0x100, 0, 0x1F2);
    TimQueueAt(KAGE_TIM(3), 0x240, 0x100, 0, 0x1F3);
    TimQueueAt(KAGE_TIM(4), 0x280, 0x100, 0, 0x1F4);
    TimQueueAt(KAGE_TIM(5), 0x2C0, 0x100, 0, 0x1F5);
    TimQueueAt(KAGE_TIM(6), 0x300, 0x100, 0, 0x1F6);
    TimQueueAt(KAGE_TIM(7), 0x340, 0x100, 0, 0x1F7);
}
