/* Persona 1 (JP) - one frame of a negotiation.  BTLP only.
 *   0x8006B09C BtlTalkSceneStep
 *
 * The negotiation is a stack of scenes, and this is what runs it: dispatch on
 * whatever is on top, then look at the phase the scene left behind, and go
 * round again until the phase says the fight is to carry on. It only ever
 * returns when the negotiation is over - zero to go back to the battle, one
 * when the battle is over too.
 *
 * Two of the scene codes have no handler of their own. Nought and thirteen
 * simply drop the level, and twelve is the sequence player: it pumps the
 * demons' voices and reads the sequence's own answer, which is how a line can
 * push the choice box on top of itself or hand over to the demand.
 *
 * The phases all end the same way - fade what is playing, shut the two sound
 * slots, and drop the effects - and differ only in what they do first and
 * what they answer. Phase three is the odd one: it rebuilds the offers and
 * only ends things if nobody is left standing.
 *
 * The case order below is the order the original's jump table lays the bodies
 * out in, and the switch table in this file's rodata is that table.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/talk.h>

/* The scenes this dispatches to. Nought and thirteen both mean "done with
   this level", and twelve is the one handled inline. */
#define TALK_SCENE_POP     0
#define TALK_SCENE_TRADE   1
#define TALK_SCENE_LEAVE   2
#define TALK_SCENE_STARE   3
#define TALK_SCENE_HAPPY   4
#define TALK_SCENE_BIND    5
#define TALK_SCENE_CHARM   6
#define TALK_SCENE_PANIC   7
#define TALK_SCENE_GIFT    9
#define TALK_SCENE_MENU    0xA
#define TALK_SCENE_SUMMON  0xB
#define TALK_SCENE_RUN     0xC
#define TALK_SCENE_POPPED  0xD
#define TALK_SCENE_ACT     0x10
#define TALK_SCENE_DEMAND  0x11

/* A level nothing is running on, and the stage a pushed one opens at. */
#define TALK_SCENE_NONE 0xFF
#define TALK_STAGE_FREE 0
#define TALK_STAGE_OPEN 1

/* What the sequence answers when it wants the choice box, and when it wants
   the demand. */
#define TALK_SEQ_DONE   0
#define TALK_SEQ_CHOICE 10
#define TALK_SEQ_HAND   0xB

/* Where the acting member's face goes while the choice box is up. */
#define TALK_FACE_X     0x3C
#define TALK_FACE_Y     0x70
#define TALK_FACE_SCALE 0x1000

/* The fade the sequence is taken out on: full volume less what it is playing
   at now, over sixty frames. */
#define TALK_FADE_FULL   0x7F
#define TALK_FADE_FRAMES 0x3C

/* The two sound slots a negotiation holds. */
#define TALK_SLOT_VOICE 3
#define TALK_SLOT_BGM   4

/* What is left in g_btl_talk_ending: two for a negotiation that simply
   finished, one for the demons joining. */
#define TALK_ENDED  2
#define TALK_JOINED 1

/* The two entries of the slot array the refresh wants cleared first. */
#define TALK_SLOT_CLEAR_A 26
#define TALK_SLOT_CLEAR_B 29

/* Set on every member still fighting once the demons have joined. */
#define BTL_ACTOR_JOINED 0x20000

extern u_char g_btl_talk_ending;
extern u_char g_btl_battle_kind;
extern short  g_btl_slot_owner[];

extern void  BtlTalkSceneTrade(void);
extern void  BtlTalkSceneLeave(void);
extern void  BtlTalkSceneStare(void);
extern void  BtlTalkSceneHappy(void);
extern void  BtlTalkSceneBind(void);
extern void  BtlTalkSceneCharm(void);
extern void  BtlTalkScenePanic(void);
extern void  BtlTalkSceneGift(void);
extern void  BtlTalkSceneMenu(void);
extern void  BtlTalkSceneSummon(void);
extern void  BtlTalkSceneAct(void);
extern void  BtlTalkSceneDemand(void);

extern void  BtlUpdateVoices(void);
extern int   BtlSeqState(void);
extern int   BtlSeqAnswer(void);
extern void  BtlOpenChoice(void);
extern void  BtlFaceLoad(int who, int always);
extern void  BtlFaceOpen(short x, short y, short scale);
extern short BtlSeqVolumeMean(void);
extern void  BtlSoundClose(int slot);
extern void  BtlBuildOffers(void);
extern int   BtlAnyStanding(void);
extern void  BtlRefreshMarkers(void);
extern void  BtlShowReadyMarkers(void);
extern int   BtlPickUpdate(short *row);
extern void  BtlEffectDrop(void);
extern void  BtlCopyMood(const u_short *mood);
extern void  BtlMenuUpdate(void);
extern void  BtlTalkUpdate(void);

int BtlTalkSceneStep(void)
{
    for (;;) {
        switch (g_btl_talk_scene[g_btl_talk_depth - 1]) {
        case TALK_SCENE_CHARM:
            BtlTalkSceneCharm();
            break;
        case TALK_SCENE_BIND:
            BtlTalkSceneBind();
            break;
        case TALK_SCENE_TRADE:
            BtlTalkSceneTrade();
            break;
        case TALK_SCENE_RUN:
            BtlUpdateVoices();
            if (BtlSeqState() == TALK_SEQ_DONE) {
                BtlEndTalking();
                g_btl_talk_depth--;
                g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
                g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
            }
            if (BtlSeqState() == TALK_SEQ_CHOICE) {
                g_btl_choice_row = BtlSeqAnswer();
                BtlOpenChoice();
                g_btl_talk_depth--;
                g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
                g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
                g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_ACT;
                g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_OPEN;
                g_btl_talk_depth++;
                BtlFaceLoad(g_btl_actors[g_btl_actor_slot].c.key, 0);
                BtlFaceOpen(TALK_FACE_X, TALK_FACE_Y, TALK_FACE_SCALE);
            }
            if (BtlSeqState() == TALK_SEQ_HAND) {
                BtlSeqRun();
                BtlEndTalking();
                g_btl_talk_depth--;
                g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
                g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
                g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_DEMAND;
                g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_OPEN;
                g_btl_talk_depth++;
            }
            break;
        case TALK_SCENE_ACT:
            BtlTalkSceneAct();
            break;
        case TALK_SCENE_DEMAND:
            BtlTalkSceneDemand();
            break;
        case TALK_SCENE_GIFT:
            BtlTalkSceneGift();
            break;
        case TALK_SCENE_MENU:
            BtlTalkSceneMenu();
            break;
        case TALK_SCENE_SUMMON:
            BtlTalkSceneSummon();
            break;
        case TALK_SCENE_PANIC:
            BtlTalkScenePanic();
            break;
        case TALK_SCENE_LEAVE:
            BtlTalkSceneLeave();
            break;
        case TALK_SCENE_STARE:
            BtlTalkSceneStare();
            break;
        case TALK_SCENE_HAPPY:
            BtlTalkSceneHappy();
            break;
        case TALK_SCENE_POP:
        case TALK_SCENE_POPPED:
            g_btl_talk_depth--;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
            break;
        }

        /* Read unsigned: the range test the switch opens with is an
           unsigned one, which a plain int phase does not give. */
        switch ((u_int)g_btl_phase) {
        case 3:
            BtlBuildOffers();
            if (BtlAnyStanding() == 0) {
                SsSepSetCrescendo(g_btl_seq[0], 0,
                                  TALK_FADE_FULL - BtlSeqVolumeMean(),
                                  TALK_FADE_FRAMES);
                g_btl_talk_ending = TALK_ENDED;
                BtlSoundClose(TALK_SLOT_VOICE);
                BtlSoundClose(TALK_SLOT_BGM);
                BtlEffectDrop();
                return 1;
            }
            /* anyone still standing and the negotiation simply hands back,
               the same way phase nought does - so it falls into it */
        case 0:
            SsSepSetCrescendo(g_btl_seq[0], 0,
                              TALK_FADE_FULL - BtlSeqVolumeMean(),
                              TALK_FADE_FRAMES);
            BtlSoundClose(TALK_SLOT_VOICE);
            BtlSoundClose(TALK_SLOT_BGM);
            g_btl_slot_owner[TALK_SLOT_CLEAR_A] = -1;
            g_btl_slot_owner[TALK_SLOT_CLEAR_B] = -1;
            BtlRefreshMarkers();
            BtlPickRefresh();
            BtlShowReadyMarkers();
            BtlPickUpdate(&g_btl_pick_help_row);
            BtlEffectDrop();
            return 0;
        case 2:
            g_btl_talk_ending = TALK_ENDED;
            SsSepSetCrescendo(g_btl_seq[0], 0,
                              TALK_FADE_FULL - BtlSeqVolumeMean(),
                              TALK_FADE_FRAMES);
            BtlSoundClose(TALK_SLOT_VOICE);
            BtlSoundClose(TALK_SLOT_BGM);
            BtlEffectDrop();
            return 1;
        case 4:
            {
                int i;
                int n;

                g_btl_talk_ending = TALK_JOINED;
                g_btl_battle_kind = TALK_ENDED;
                SsSepSetCrescendo(g_btl_seq[0], 0,
                                      TALK_FADE_FULL - BtlSeqVolumeMean(),
                                      TALK_FADE_FRAMES);
                BtlSoundClose(TALK_SLOT_VOICE);
                BtlSoundClose(TALK_SLOT_BGM);
                n = 0;
                i = 0;
                do {
                    n++;
                    if (g_btl_actors[i].c.key != 0
                        && (signed char)g_btl_actors[i].c.status
                               != BTL_STATUS_DOWN
                        && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                        g_btl_actors[i].flags |= BTL_ACTOR_JOINED;
                    }
                    i++;
                } while (n < BTL_PARTY);
                BtlRefreshMarkers();
                BtlEffectDrop();
                return 1;
            }
        }

        BtlCopyMood((const u_short *)g_btl_offer[g_btl_offer_slot].mood);
        BtlMenuUpdate();
        BtlTalkUpdate();
        BtlDrawFrame();
    }
}
