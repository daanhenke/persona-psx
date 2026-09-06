/* Persona 1 (JP) - the fight ending because the negotiation did.  BTLP only.
 *   0x800856C4 BtlTalkLeave
 *
 * Three things at once: the scene tint goes back to neutral, the demon is put
 * on the motion that takes it off the field, and the battle music is closed.
 * Callers wait afterwards for the object to report itself finished, which is
 * what holds the fight open until the demon has actually gone.
 */
#include <types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/sound.h>

/* Neutral on each channel. */
#define SCENE_RGB_NEUTRAL 0x80

/* The motion the demon leaves on. */
#define BTL_MOTION_LEAVE 4

extern short g_btl_scene_rgb[];

extern void BtlObjSetMotion(BtlObj *obj, u_char motion);
extern void BtlObjSetPhase(BtlObj *obj, u_char phase);

void BtlTalkLeave(BtlObj *obj)
{
    g_btl_scene_rgb[0] = SCENE_RGB_NEUTRAL;
    g_btl_scene_rgb[1] = SCENE_RGB_NEUTRAL;
    g_btl_scene_rgb[2] = SCENE_RGB_NEUTRAL;
    BtlObjSetMotion(obj, BTL_MOTION_LEAVE);
    BtlObjSetPhase(obj, 0);
    BtlSoundClose(BTL_BGM_SLOT);
}
