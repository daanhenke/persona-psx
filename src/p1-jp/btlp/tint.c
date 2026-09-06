/* Persona 1 (JP) - lighting the enemies a negotiation can reach.  BTLP only.
 *   0x8006BEDC BtlTintTalkers
 *
 * Every enemy still on the field goes black, except the ones this offer was
 * put to - those come up to a dim grey, and the one the negotiation is aimed
 * at to full. It reads the offer's whole set rather than the narrowed one, so
 * a demon that has since been afflicted stays lit. The colours are set as a
 * target and reached through the fade, four steps a frame, so what the player
 * sees is a ramp rather than a cut.
 *
 * The motion is reset at the same time, which is what stops a demon in the
 * middle of an animation from carrying it into the conversation.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/offer.h>

/* Black, the answering demons, and the one being spoken to. */
#define TINT_DARK  0
#define TINT_DIM   0x38
#define TINT_FULL  0x80

/* Steps a frame the tint is reached in. */
#define TINT_FADE  4

extern BtlActor g_btl_enemies[];
extern short    g_btl_offer_slot;
extern short    g_btl_talk_target;

extern void BtlObjSetMotion(BtlObj *obj, u_char motion);
extern void BtlObjSetRgb(BtlObj *obj, int r, int g, int b);
extern void BtlObjSetFade(BtlObj *obj, u_char fade);

void BtlTintTalkers(void)
{
    BtlActor *actor;
    int       slot;
    int       tint;

    slot = 0;
    actor = g_btl_enemies;
    do {
        if (actor->c.key != 0) {
            tint = TINT_DARK;
            if (*(signed char *)&actor->c.status == 0 &&
                (g_btl_offer[g_btl_offer_slot].used >> slot & 1) != 0) {
                tint = TINT_DIM;
                if (slot == g_btl_talk_target) {
                    tint = TINT_FULL;
                }
            }
            BtlObjSetMotion(actor->obj, 0);
            BtlObjSetRgb(actor->obj, tint, tint, tint);
            BtlObjSetFade(actor->obj, TINT_FADE);
        }
        slot++;
        actor++;
    } while (slot < BTL_ENEMIES);
}
