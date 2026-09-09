/* Persona 1 (JP) - the party takes the Persona a demon offered.  BTLP only.
 *   0x800673D4 BtlTakeOffer
 *
 * The id goes into the stock and into the compendium's record of what has been
 * won; the offer carries the Persona's name at +0x31 and the arcana comes out
 * of g_btl_arcana_names, and both are dropped into the message before it is
 * said. The caller's line runs first, then the one the demon leaves on.
 *
 * Collecting every Persona is checked here rather than anywhere else: the
 * flags are read from one upward until one comes back clear, and reaching 0x8F
 * with 0x8F itself still clear means the set is complete. The reward is item
 * 0x7B, handed over with a short scene - a shared line, the acting member's
 * face and their own line - and flag 0x8F is then set so it happens once. With
 * no room for the item nothing happens and the check comes round again.
 *
 * Either way the negotiation is taken down and the battle moves on.
 */
#include <decomp/types.h>
#include <persona/common/persona.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/common/item.h>

/* Message slots the two inserts fill in. */
#define TAKE_SLOT_NAME   2
#define TAKE_SLOT_ARCANA 6

/* Personas, and the flag that says the reward for all of them has been given. */
#define BTL_PERSONA_FLAGS 0x8F

/* What completing the compendium is worth. */
#define BTL_REWARD_ITEM 0x7B

/* Where the reward scene puts the face. */
#define REWARD_FACE_X     0x3C
#define REWARD_FACE_Y     0x70
#define REWARD_FACE_SCALE 0x1000

/* The battle goes back to this once a Persona has been taken. */
#define BTL_PHASE_TAKEN 2

extern const u_char *g_btl_arcana_names[];
extern const u_char *g_btl_talk_joined_script;
extern const u_char *g_btl_reward_scripts[];

extern void  BtlStockAdd(int persona);
extern void  BtlNotePersonaWon(int persona);
extern void  BtlSetInsert(int slot, const u_char *text);
extern void  BtlBeginAction(void);
extern void  BtlFlagSet(int flag);
extern int   BtlFlagGet(short flag);
extern void  BtlFaceLoad(int who, int side);
extern void  BtlFaceOpen(int x, int y, int scale);
extern void  BtlShowAilmentMarks(int show);

void BtlTakeOffer(u_short persona, const u_char *script)
{
    int flag;

    BtlStockAdd(persona & 0xFF);
    BtlNotePersonaWon((short)persona);
    BtlSetInsert(TAKE_SLOT_NAME, &g_btl_offer[g_btl_offer_slot].name[0]);
    BtlSetInsert(TAKE_SLOT_ARCANA,
                 g_btl_arcana_names[g_persona_data[
                     g_btl_offer[g_btl_offer_slot].persona].arcana]);
    BtlSeqPlay(script);
    flag = 1;
    BtlBeginAction();
    BtlSeqRun();
    BtlSeqPlay(g_btl_talk_joined_script);
    BtlFlagSet((short)persona);
    while (BtlFlagGet(flag) != 0) {
        flag++;
        if (flag >= BTL_PERSONA_FLAGS) {
            break;
        }
    }
    if (BtlFlagGet(BTL_PERSONA_FLAGS) == 0 && flag == BTL_PERSONA_FLAGS
        && (u_short)BtlItemSlot(BTL_REWARD_ITEM) != 0) {
        BtlSeqRun();
        BtlSeqPlay(g_btl_reward_scripts[0]);
        BtlFaceLoad(g_btl_actors[g_btl_actor_slot].c.key, 0);
        BtlFaceOpen(REWARD_FACE_X, REWARD_FACE_Y, REWARD_FACE_SCALE);
        BtlSeqRun();
        BtlSeqPlay(g_btl_reward_scripts[g_btl_actors[g_btl_actor_slot].c.key]);
        BtlItemAdd(BTL_REWARD_ITEM);
        BtlFlagSet(BTL_PERSONA_FLAGS);
    }
    BtlSeqWaitDone();
    BtlShowAilmentMarks(1);
    BtlFaceClose();
    BtlPanelClose();
    BtlBoxClose();
    BtlSeqClear();
    BtlHudHide();
    BtlEnemiesReset();
    BtlPartyReset();
    g_btl_phase = BTL_PHASE_TAKEN;
}
