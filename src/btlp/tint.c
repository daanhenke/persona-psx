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
#include <decomp/types.h>
#include <libetc.h>
#include <rand.h>
#include <persona/common/persona.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>

/* Black, the answering demons, and the one being spoken to. */
#define TINT_DARK  0
#define TINT_DIM   0x38
#define TINT_FULL  0x80

/* Steps a frame the tint is reached in. */
#define TINT_FADE  4

/* The party's tints, and the two scripts a speaker can be put on. */
#define TINT_PARTY_DIM   0x20
#define TINT_PARTY_FULL  0x80
#define TINT_PARTY_FADE  4
#define TINT_SPEAK_FADE  0x10
#define TINT_SCRIPT_ALT  2
#define TINT_SCRIPT_A    (0x8C / 4)
#define TINT_SCRIPT_B    (0x0C / 4)

void BtlTintParty(void)
{
    BtlActor *actor;
    BtlActor *party;
    BtlObj   *obj;
    int       slot;

    slot = 0;
    actor = g_btl_actors;
    do {
        if (actor->c.key != 0) {
            if (slot == g_btl_actor_slot) {
                BtlObjSetRgb(actor->obj, TINT_PARTY_FULL, TINT_PARTY_FULL,
                             TINT_PARTY_FULL);
                BtlObjSetFade(actor->obj, TINT_SPEAK_FADE);
            } else {
                BtlObjSetRgb(actor->obj, TINT_PARTY_DIM, TINT_PARTY_DIM,
                             TINT_PARTY_DIM);
                BtlObjSetFade(actor->obj, TINT_PARTY_FADE);
            }
        }
        slot++;
        actor++;
    } while (slot < BTL_PARTY);
    party = g_btl_actors;
    if (party[g_btl_actor_slot].script_pick == TINT_SCRIPT_ALT) {
        obj = party[g_btl_actor_slot].obj;
        BtlObjSetScript(obj, obj->scripts[TINT_SCRIPT_A]);
    } else {
        obj = party[g_btl_actor_slot].obj;
        BtlObjSetScript(obj, obj->scripts[TINT_SCRIPT_B]);
    }
}

/* Twelve random Personas into the stock, seeded from the frame counter. There
   are no callers: it is a debug hand for testing the stock screen, left in the
   build. Ids run 2 to 101, which is why the roll is offset. */
void BtlStockRandomise(void)
{
    u_char *p;
    int     i;

    p = g_persona_stock;
    srand(VSync(-1));
    i = 0;
    do {
        i++;
        *p = rand() % STOCK_RANDOM_RANGE + STOCK_RANDOM_FIRST;
        p++;
    } while (i < STOCK_ROWS);
}
