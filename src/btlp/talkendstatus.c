/* Persona 1 (JP) - what the demons are left in when a contact ends.
 *   0x8006C170 BtlTalkEndStatus    BTLP only.
 *
 * A negotiation can end four ways and each calls this with its own ailment
 * code and pack entry - 1 with 0xE, 2 with 0xB, 3 with 0xD, 6 with 0xC - so
 * the ending has both a sound of its own and a state it leaves the demons in.
 *
 * The pack is read off the disc and played on the BGM slot before anything
 * moves. Then every demon the offer involved that is still free of an ailment
 * takes the code, is put on ailment level 1, has its marker re-armed and is
 * brightened back to plain white over sixteen frames. The screen is held after
 * each one so they change one at a time rather than all together: 0x19 frames
 * for the first and five to nine for the ones after it.
 */
#include <decomp/types.h>
#include <rand.h>
#include <libsnd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/status.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/battle.h>

/* The level a demon's fresh ailment is put on as a contact ends. */
#define TALK_END_LEVEL 1

/* The slot the pack is played on, and the sound that starts it. */
#define TALK_END_SE 0

/* Frames before the first demon changes, and while it does. */
#define TALK_END_LEAD 10
#define TALK_END_HOLD 0x19

/* The gap between the ones after it: five frames plus a coin. */
#define TALK_END_GAP  5

/* Plain white, reached over sixteen frames. */
#define TALK_END_WHITE 0x80
#define TALK_END_FADE  0x10

extern volatile long g_cd_busy;

extern void  BtlLoadPackEntry(int entry);
extern void  BtlBgmOpen(void);
extern short BtlPickTalkTarget(short mask);

void BtlTalkEndStatus(u_char status, int pack)
{
    BtlActor *e;
    int       used;
    int       target;
    int       wait;

    used = g_btl_offer[g_btl_offer_slot].used;
    BtlLoadPackEntry(pack);
    while (g_cd_busy != -1) {
        BtlDrawFrame();
    }
    wait = TALK_END_HOLD;
    BtlBgmOpen();
    SsVabTransCompleted(1);
    BtlSePlay(BTL_BGM_SLOT, TALK_END_SE);
    BtlRunFrames(TALK_END_LEAD);

    for (;;) {
        target = BtlPickTalkTarget(used);
        if (target == -1) {
            break;
        }
        e = &g_btl_enemies[target];
        if (*(signed char *)&e->c.status == 0) {
            e->c.ail_level = TALK_END_LEVEL;
            e->c.status = status;
            e->ail_turns = 2;
            BtlObjSetScript(e->obj->mark,
                            *(const u_long **)(g_btl_actor_gfx + status * 4
                                               + BTL_GFX_SCRIPTS));
            BtlObjSetScript(e->obj->mark->attached,
                            g_btl_ail_level_marks[*(signed char *)&e->c.ail_level]);
            e->obj->mark->unkCE = status + BTL_MARK_BIAS;
            BtlObjSetRgb(e->obj, TALK_END_WHITE, TALK_END_WHITE, TALK_END_WHITE);
            BtlObjSetFade(e->obj, TALK_END_FADE);
            while (--wait != -1) {
                BtlShowAilmentMarks(1);
                BtlDrawFrame();
            }
            wait = rand() % TALK_END_GAP + TALK_END_GAP;
        }
        used &= ~(1 << target);
    }
    BtlSoundClose(BTL_BGM_SLOT);
}

/* Empty in the shipped build. All four ways a contact can end call it once
   the player has acknowledged the closing message. */
void BtlTalkEndStep(void)
{
}

