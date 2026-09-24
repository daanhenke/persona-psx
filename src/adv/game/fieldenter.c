/* Persona 1 (JP) - building a field scene.  ADV only.
 *   0x8007E744 AdvFieldEnter
 *
 * The first field frame puts the room up from what the scene pack says: the
 * picture's cell index, the command bar parked above the screen, the room's
 * actors, the scene's own images and the camera on the leader. A scene coming
 * back from a script that left for ADVCMD keeps the actors it had.
 */
#include <decomp/types.h>
#include <persona/adv/actor.h>
#include <persona/adv/room.h>
#include <persona/adv/scene.h>
#include <persona/common/slot.h>

#define g_script_resuming (*(u_char *)0x801F1B88)

#define BAR_SLOT  0x29
#define BAR_SLOT2 0x2A
#define BAR_HIDE  -0x20

extern u_long g_bg_shown;
extern u_char g_bar_def[];
extern u_char g_bar_def2[];

extern void SlotClearAll(void);
extern void SlotInit(void *def, u_char slot, int attr, short x, short y);
extern void AdvBuildActors(void);
extern void AdvSceneStartImages(void);
extern void CamCenterOnActor(u_char actor);
extern void ActorsPlaceSprites(void);
extern void SlotsApplyXScale(void);
extern void func_80084694(void);

void AdvFieldEnter(void)
{
    g_bg_shown = 1;
    ImageIndexInit(g_adv_scene->kind);
    SlotClearAll();
    SlotInitTagged(g_bar_def, BAR_SLOT, 0x35, 0xE0, BAR_HIDE);
    SlotInit(g_bar_def2, BAR_SLOT2, 0x34, 0xE3, BAR_HIDE);
    if (!g_script_resuming) {
        AdvBuildActors();
    }
    AdvSceneStartImages();
    func_80084694();
    CamCenterOnActor(0);
    /* A stray second argument, as tyncut.c has it too. */
    ((void (*)())ActorsSetDepth)(0, 0xF);
    ActorsPlaceSprites();
    SlotsApplyXScale();
}
