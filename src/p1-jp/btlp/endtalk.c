/* Persona 1 (JP) - putting the field back after a talk action.  BTLP only.
 *   0x8006C3A0 BtlEndTalking
 *
 * Thirteen places call this the moment a negotiation action finishes, and it
 * undoes all of it: every enemy the offer involves goes back to the pose it
 * stands in, which is its model's spawn script rather than a script of its
 * own, so each demon returns to its own idle; the voice bank's ten
 * sub-sequences are stopped whether or not they were playing; and the target
 * is picked again from the same mask, since the demon that speaks for the
 * group can have changed while the action ran.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/offer.h>
#include <psyq/libsnd.h>

/* The bank the demon voices are opened in, and its sub-sequences. */
#define BTL_VOICE_BANK 3
#define BTL_VOICE_SEPS 10

extern BtlActor g_btl_enemies[];
extern short    g_btl_offer_slot;
extern short    g_btl_talk_target;
extern short    g_btl_voice_kind;
extern short    g_btl_seq[];

extern void  BtlObjSetScript(BtlObj *obj, const u_long *script);
extern short BtlPickTalkTarget(short mask);
extern void  BtlTintTalkers(void);
extern void  BtlHighlightEnd(void);

void BtlEndTalking(void)
{
    BtlActor *e;
    int       i;

    i = 0;
    e = g_btl_enemies;
    do {
        if (e->c.key != 0
            && ((g_btl_offer[g_btl_offer_slot].used >> i) & 1) != 0) {
            BtlObjSetScript(e->obj,
                            e->obj->scripts[g_btl_models[e->c.key].spawn]);
        }
        i++;
        e++;
    } while (i < BTL_ENEMIES);

    i = 0;
    do {
        SsSepStop(g_btl_seq[BTL_VOICE_BANK], i);
        i++;
    } while (i < BTL_VOICE_SEPS);

    g_btl_voice_kind = 0;
    g_btl_talk_target = BtlPickTalkTarget(g_btl_offer[g_btl_offer_slot].used);
    BtlTintTalkers();
    BtlHighlightEnd();
}
