/* Persona 1 (JP) - the talk's own menus.  BTLP only.
 *   0x8006C964 BtlTalkSceneMenu
 *
 * BtlTalkSceneStep runs this while the top of g_btl_talk_scene is the menu,
 * and the stage below the top says where in it the player is:
 *
 *   1  find the first offer that involves anybody and put the panel up
 *   3  pick an offer. Cancelling takes the whole talk down; an offer that has
 *      been contacted already, or has nobody left who can talk, only buzzes.
 *      A demon whose profile does not wait for the party plays its own scene
 *      and the talk ends there; the rest move on to 5
 *   5  if the party already holds the Persona on offer, a member who can hear
 *      it says so first; then 7
 *   7  pick the member who talks. Cancelling goes back to 3; picking a member
 *      with something to say pushes their scene and waits at 8
 *   8  wait out the disc, then 9
 *   9  the member's face and contact box come up; then 10
 *   10 pick the act. Cancelling goes back to 7
 *   11 the act lands: the demon's reaction moves a gauge, and a filled gauge
 *      the offer wants ends the talk in BtlTalkAnswer
 *   20 the demons walk off with a line, and then as 2
 *   2  the talk is over and the battle carries on as won
 *   4  the talk is over and the battle is lost
 *
 * Whatever happens in 3, the panel is brought up to date with the offer under
 * the cursor on the way out: its gauges, the gauge it was last pushed on, and
 * its picture.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/box.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/face.h>
#include <persona/btlp/hud.h>
#include <persona/btlp/member.h>
#include <persona/btlp/message.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/panel.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/status.h>
#include <persona/btlp/talk.h>
#include <persona/btlp/text.h>
#include <persona/common/persona.h>
#include <persona/main/cd.h>

/* The stages, as the list above has them. */
#define MENU_FIND     1
#define MENU_WON      2
#define MENU_OFFER    3
#define MENU_LOST     4
#define MENU_HELD     5
#define MENU_ASIDE    6
#define MENU_MEMBER   7
#define MENU_DISC     8
#define MENU_FACE     9
#define MENU_ACT      10
#define MENU_LANDS    11
#define MENU_LEAVE    20

/* The scenes pushed on top of the menu. */
#define TALK_SCENE_NONE   0xFF
#define TALK_SCENE_HELD   9
#define TALK_SCENE_MEMBER 0xB

/* What the pickers answer while nothing is chosen, and on cancel. */
#define PICK_WAIT   (-0x100)
#define PICK_CANCEL (-1)

/* A profile with this bit waits for the party rather than playing a scene. */
#define PROFILE_WAITS 0x8000

/* Where the boxes go. */
#define TALK_TEXT_X   0x28
#define TALK_OFFER_Y  0x9C
#define TALK_MEMBER_Y 0x92
#define TALK_BOX_COLS 0x11
#define TALK_BOX_X    0xA0

/* The file the persona scenes are read from, and the pause before a win. */
#define TALK_SCENE_FILE 0x1C
#define TALK_WIN_WAIT   0x1E

/* A gauge is pushed by 0x23 and the weight the reaction carries, and is never
   left above BTL_MOOD_CAP. A mood state counts up to five. */
#define REACTION_PUSH  0x23
#define BTL_MOOD_CAP   0x5F
#define MOOD_STATE_MAX 5

/* The sound slot the talk's voices were read into. */
#define TALK_VOICE_SLOT 3

/* Knocks the talk down a level and puts everything it had up away again. */
#define TALK_MENU_CLOSE()                                        \
    g_btl_talk_depth--;                                          \
    g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;        \
    g_btl_talk_stage[g_btl_talk_depth] = 0;                      \
    BtlShowAilmentMarks(1);                                      \
    BtlFaceClose();                                              \
    BtlPanelClose();                                             \
    BtlBoxClose();                                               \
    BtlSeqClear();                                               \
    BtlHudHide();                                                \
    BtlEnemiesReset();                                           \
    BtlPartyReset()

/* Not matched yet: everything but stage 11's push. The image adds the reaction
   to the gauge through a pointer, so the store invalidates g_btl_talk_reaction
   and BtlTalkPersonaBonus's argument is read again; the indexed store below
   lets cse hand it the push's own `& 0xFF`. Written through a pointer, gcc
   reads the argument again but schedules the index before the said and
   last-line stores rather than after them. */
#ifdef NON_MATCHING
void BtlTalkSceneMenu(void)
{
    BtlOffer *o;
    BtlActor *a;
    int       choice;
    int       rank;
    int       state;
    int       line;
    int       off;
    short     g;
    int       i;

    switch (g_btl_talk_stage[g_btl_talk_depth - 1]) {
    case MENU_FIND:
        for (i = 0; i < BTL_OFFERS; i++) {
            if (g_btl_offer[i].used != 0) {
                break;
            }
        }
        g_btl_panel_offer = -1;
        g_btl_offer_slot = i;
        g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_OFFER;
        BtlPanelLoad();
        BtlPanelOpen();
        break;

    case MENU_OFFER:
        for (i = 0; i < BTL_ENEMIES; i++) {
            g_btl_actors[i + BTL_PARTY].pickable = 1;
        }
        switch ((short)BtlPickOffer(&g_btl_offer_slot)) {
        case PICK_WAIT:
            if ((g_btl_offer[g_btl_offer_slot].kinds & OFFER_CONTACTED) != 0
                || g_btl_offer[g_btl_offer_slot].talkers == 0) {
                BtlTextOpen(g_btl_talk_unpickable_script, TALK_TEXT_X, TALK_OFFER_Y);
                BtlBoxOpen(TALK_BOX_COLS, TALK_BOX_X, TALK_OFFER_Y, 0);
            } else {
                BtlBoxClose();
            }
            BtlOfferMenu(g_btl_offer_slot);
            break;
        case PICK_CANCEL:
            BtlTalkMenuEndEffect();
            BtlPanelClose();
            BtlSePlay(1, 2);
            BtlBoxClose();
            TALK_MENU_CLOSE();
            g_btl_phase = 0;
            break;
        default:
            o = &g_btl_offer[g_btl_offer_slot];
            if ((g_btl_offer[g_btl_offer_slot].kinds & OFFER_CONTACTED) != 0
                || g_btl_offer[g_btl_offer_slot].talkers == 0) {
                BtlSePlay(1, 2);
                break;
            }
            BtlBoxClose();
            BtlTalkMenuEndEffect();
            BtlSePlay(1, 1);
            BtlSetInsert(2, g_btl_offer[g_btl_offer_slot].name);
            BtlSetInsert(6, g_btl_arcana_names[g_persona_data[
                                g_btl_offer[g_btl_offer_slot].persona].arcana]);
            if ((((u_short *)g_btl_demon_talk_profiles[o->persona])[1] & PROFILE_WAITS) == 0) {
                BtlLoadPackBank(g_btl_offer[g_btl_offer_slot].persona);
                BtlHudLoad();
                BtlHudShow();
                BtlSeekFile(TALK_SCENE_FILE);
                BtlLoadScratch(0, 0);
                while (SsVabTransCompleted(0) == 0) {
                    BtlDrawFrame();
                }
                while (g_cd_busy != -1) {
                    BtlDrawFrame();
                }
                while (BtlHudState() != 0) {
                    BtlDrawFrame();
                }
                for (i = 0; g_btl_talk_persona_scenes[i].persona != o->persona; i++) {
                }
                g_btl_talk_target = BtlPickTalkTarget(g_btl_offer[g_btl_offer_slot].used);
                BtlTintTalkers();
                BtlSeqPlay(BTL_SCRATCH + *(u_long *)BTL_SCRATCH
                           + *(u_long *)(BTL_SCRATCH + *(u_long *)BTL_SCRATCH
                                         + g_btl_talk_persona_scenes[i].entry * 4));
                BtlSeqWaitDone();
                TALK_MENU_CLOSE();
                g_btl_phase = 0;
            } else {
                BtlHudLoad();
                BtlHudShow();
                g_btl_talk_target = BtlPickTalkTarget(g_btl_offer[g_btl_offer_slot].used);
                BtlTintTalkers();
                BtlLoadPackBank(g_btl_offer[g_btl_offer_slot].persona);
                BtlSeekFile(g_btl_offer[g_btl_offer_slot].voice);
                SsVabTransCompleted(1);
                g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_HELD;
            }
            break;
        }
        g_btl_panel_gauges = (u_short)g_btl_offer[g_btl_offer_slot].kinds & 0xF;
        for (i = 0; i < BTL_MOODS; i++) {
            g_btl_recent[i] = -1;
        }
        for (i = 0; i < BTL_MOODS; i++) {
            if (((short)g_btl_panel_gauges >> i & 1) != 0) {
                g_btl_recent[0] = i;
                g_btl_recent[1] = -1;
                g_btl_recent[2] = -1;
                g_btl_recent[3] = -1;
                break;
            }
        }
        if ((g_btl_offer[g_btl_offer_slot].kinds & OFFER_CONTACTED) != 0) {
            BtlPanelShowOffer(g_btl_offer_slot);
        } else if ((g = g_btl_panel_gauges) != 0) {
            BtlPanelSetImage(1, g_btl_panel_gauges);
        } else {
            BtlPanelSetImage(0, 0);
        }
        BtlSetMoodGauges(g_btl_offer[g_btl_offer_slot].mood[0],
                         g_btl_offer[g_btl_offer_slot].mood[1],
                         g_btl_offer[g_btl_offer_slot].mood[2],
                         g_btl_offer[g_btl_offer_slot].mood[3]);
        g_btl_panel_offer = g_btl_offer_slot;
        break;

    case MENU_LOST:
        TALK_MENU_CLOSE();
        g_btl_phase = 2;
        break;

    case MENU_HELD:
        if (BtlStockHolds(&g_btl_offer[g_btl_offer_slot]) == 0) {
            g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_MEMBER;
            break;
        }
        BtlLoadScratch(g_btl_talk_member_pack, 0);
        while (g_cd_busy != -1) {
            BtlDrawFrame();
        }
        BtlSeqPlay(BTL_SCRATCH + *(u_long *)BTL_SCRATCH
                   + *(u_long *)(BTL_SCRATCH + *(u_long *)BTL_SCRATCH
                                 + ((u_short *)g_btl_scratch_end)[0] * 4));
        g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_LEAVE;
        a = g_btl_actors;
        for (i = 0; i < BTL_PARTY; i++) {
            if (a->c.key == 1) {
                break;
            }
            a++;
        }
        if ((rand() & 8) == 0 && i != BTL_PARTY) {
        g_btl_actor_slot = i;
        BtlSetInsert(4, g_btl_actors[g_btl_actor_slot].c.name);
        BtlSeqRun();
        BtlSeqPlay(BTL_SCRATCH + *(u_long *)BTL_SCRATCH
                   + *(u_long *)(BTL_SCRATCH + *(u_long *)BTL_SCRATCH
                                 + ((u_short *)g_btl_scratch_end)[1] * 4));
        BtlSeqWaitDone();
        g_btl_talk_pair = 7;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_HELD;
        g_btl_talk_stage[g_btl_talk_depth] = 1;
        g_btl_talk_depth++;
        break;
        }
        BtlSeqWaitDone();
        break;

    case MENU_MEMBER:
        if (BtlActorIsDown(g_btl_actor_slot) == 1) {
            for (i = 0; i < BTL_PARTY; i++) {
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & 0x4000) == 0) {
                    g_btl_actor_slot = i;
                    break;
                }
            }
        }
        for (i = 0; i < BTL_PARTY; i++) {
            g_btl_actors[i].pickable = 1;
        }
        while (BtlHudState() != 0) {
            BtlDrawFrame();
        }
        BtlDrawFrame();
        BtlTalkOpen(g_btl_actors[g_btl_actor_slot].c.key,
                    g_btl_actors[g_btl_actor_slot].c.level,
                    (const char *)g_btl_actors[g_btl_actor_slot].c.name);
        g_btl_talk_open_slot = g_btl_actor_slot;
        if (g_btl_member[g_btl_actor_slot].answer == 0) {
            BtlTextOpen(g_btl_talk_mute_script, TALK_TEXT_X, TALK_MEMBER_Y);
            BtlBoxOpen(TALK_BOX_COLS, TALK_BOX_X, TALK_MEMBER_Y, 0);
        } else {
            BtlBoxClose();
        }
        i = 1;
        BtlDrawFrame();
        BtlDrawFrame();
        do {
            switch (BtlPickMember(&g_btl_actor_slot)) {
            case PICK_WAIT:
                if (g_btl_talk_open_slot != g_btl_actor_slot) {
                    BtlTalkOpen(g_btl_actors[g_btl_actor_slot].c.key,
                                g_btl_actors[g_btl_actor_slot].c.level,
                                (const char *)g_btl_actors[g_btl_actor_slot].c.name);
                    g_btl_talk_open_slot = g_btl_actor_slot;
                }
                if (g_btl_member[g_btl_actor_slot].answer == 0) {
                    BtlTextOpen(g_btl_talk_mute_script, TALK_TEXT_X, TALK_MEMBER_Y);
                    BtlBoxOpen(TALK_BOX_COLS, TALK_BOX_X, TALK_MEMBER_Y, 0);
                } else {
                    BtlBoxClose();
                }
                break;
            case PICK_CANCEL:
                if ((g_btl_offer[g_btl_offer_slot].kinds & OFFER_TALKING) != 0) {
                    BtlSePlay(1, 2);
                    g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_ASIDE;
                    break;
                }
                BtlSePlay(1, 2);
                g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_OFFER;
                i = 0;
                BtlHudHide();
                BtlTalkHide();
                BtlBoxClose();
                BtlSoundClose(TALK_VOICE_SLOT);
                a = g_btl_enemies;
                for (i = 0; i < BTL_ENEMIES; i++, a++) {
                    if (a->c.key != 0) {
                        a->obj->rgb[0] = 0x80;
                        a->obj->rgb[1] = 0x80;
                        a->obj->rgb[2] = 0x80;
                        a->obj->attr |= 0x400000;
                    }
                }
                i = 0;
                break;
            default:
                if (g_btl_member[g_btl_actor_slot].answer != 0) {
                    i = 0;
                    BtlBoxClose();
                    BtlShowAilmentMarks(0);
                    BtlSePlay(1, 1);
                    BtlLoadScratch(g_btl_talk_member_pack, 0);
                    BtlSetInsert(4, g_btl_actors[g_btl_actor_slot].c.name);
                    g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_DISC;
                    BtlTintParty();
                    g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_MEMBER;
                    g_btl_talk_stage[g_btl_talk_depth] = 1;
                    g_btl_talk_depth++;
                }
                break;
            }
            BtlDrawFrame();
        } while (i != 0);
        break;

    case MENU_DISC:
        while (g_cd_busy != -1) {
            BtlDrawFrame();
        }
        g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_FACE;
        break;

    case MENU_FACE:
        BtlFaceLoad(g_btl_actors[g_btl_actor_slot].c.key, 0);
        BtlFaceOpen(0x3C, 0x70, 0x1000);
        BtlSeekPackEntry(g_btl_actors[g_btl_actor_slot].c.key);
        g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_ACT;
        BtlSeqReset();
        BtlTalkOpen(g_btl_actors[g_btl_actor_slot].c.key,
                    g_btl_actors[g_btl_actor_slot].c.level,
                    (const char *)g_btl_actors[g_btl_actor_slot].c.name);
        BtlRunFrames(0xF);
        g_btl_talk_open_slot = g_btl_actor_slot;
        BtlTalkLive(g_btl_talk_reply);
        BtlPanelOpen();
        BtlSetMoodGauges(g_btl_offer[g_btl_offer_slot].mood[0],
                         g_btl_offer[g_btl_offer_slot].mood[1],
                         g_btl_offer[g_btl_offer_slot].mood[2],
                         g_btl_offer[g_btl_offer_slot].mood[3]);
        /* Each bar is the first offer's gauge moved along by the slot's
           record. Through &mood[k] of the slot's own record, C adds k to
           the record's address after the fact and the constants never fold. */
        off = g_btl_offer_slot * sizeof(BtlOffer);
        g_btl_mood_bar[0] = (short *)((char *)&g_btl_offer[0].mood[0] + off);
        g_btl_mood_bar[1] = (short *)((char *)&g_btl_offer[0].mood[1] + off);
        g_btl_mood_bar[2] = (short *)((char *)&g_btl_offer[0].mood[2] + off);
        g_btl_mood_bar[3] = (short *)((char *)&g_btl_offer[0].mood[3] + off);
        break;

    case MENU_ACT:
        choice = BtlTalkChoice();
        if (choice == PICK_WAIT) {
            break;
        }
        if (choice == PICK_CANCEL) {
            BtlShowAilmentMarks(1);
            BtlSePlay(1, 2);
            BtlPartyReset();
            BtlFaceClose();
            BtlTalkIdle();
            g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_MEMBER;
            break;
        }
        line = g_btl_member[g_btl_actor_slot].key;
        line--;
        g_btl_talk_picked = BtlTalkChoice();
        g_btl_talk_reply = BtlTalkChoice();
        line <<= 2;
        line += (u_short)g_btl_talk_picked;
        g_btl_talk_line = line;
        BtlTalkHide();
        BtlRunFrames(10);
        g_btl_offer[g_btl_offer_slot].kinds |= OFFER_TALKING;
        g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_LANDS;
        break;

    case MENU_LANDS:
        *(u_char *)&g_btl_battle_kind = 0;
        g_btl_talking = 1;
        if (BtlTalkLineLands() == 1) {
            g_btl_talk_reaction = g_btl_talk_said;
        } else {
            g_btl_talk_reaction = (u_short)BtlPickReaction(g_btl_talk_picked);
        }
        g_btl_mood_before = *(BtlMoods *)g_btl_offer[g_btl_offer_slot].mood;
        for (i = 0; i < BTL_MOODS; i++) {
            if (g_btl_force_reaction[i] == 1) {
                g_btl_talk_reaction = i;
            }
        }
        g_btl_talk_said = (g_btl_talk_reaction & 0xFF);
        g_btl_talk_last_line = g_btl_talk_line;
        g_btl_offer[g_btl_offer_slot].mood[g_btl_talk_reaction & 0xFF]
            += REACTION_PUSH + ((g_btl_talk_reaction & 0xFF00) >> 8);
        BtlTalkPersonaBonus((g_btl_talk_reaction & 0xFF));
        BtlTalkLikedEquip();
        for (i = 0; i < BTL_MOODS; i++) {
            if (g_btl_offer[g_btl_offer_slot].mood[i] > BTL_MOOD_CAP) {
                g_btl_offer[g_btl_offer_slot].mood[i] = BTL_MOOD_CAP;
            }
        }
        for (i = 0; i < BTL_MOODS; i++) {
            if (g_btl_mood_state[i] != 0) {
                state = g_btl_mood_state[i] + 1;
                if (state > MOOD_STATE_MAX) {
                    state = MOOD_STATE_MAX;
                }
                g_btl_mood_state[i] = state;
            }
        }
        rank = BtlOfferRank(g_btl_offer_slot);
        if (rank == 1) {
            if (((short)g_btl_panel_gauges >> (g_btl_talk_reaction & 0xFF) & 1) != 0
                && ((1 << (g_btl_talk_reaction & 0xFF))
                    & g_btl_offer[g_btl_offer_slot].kinds) != 0) {
                BtlRunFrames(TALK_WIN_WAIT);
                BtlSeqPlay(g_btl_talk_win_scripts[g_btl_actors[g_btl_actor_slot].c.key]
                                                 [g_btl_talk_picked]);
                BtlSeqWaitDone();
                BtlFaceClose();
                BtlSeqStart();
                BtlSeqWaitDone();
                BtlQueueVoice((u_char)g_btl_talk_said, 0);
                g_btl_talk_depth--;
                g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
                g_btl_talk_stage[g_btl_talk_depth] = 0;
                BtlTalkAnswer(g_btl_offer_slot, g_btl_talk_reaction);
                break;
            }
            g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_FACE;
            line = BtlPickLine(g_btl_talk_picked);
            BtlBgmChange(g_btl_actors[g_btl_actor_slot].c.key, g_btl_talk_picked, line);
            g_btl_talk_line_script = g_btl_talk_scripts[g_btl_talk_script_ids
                [g_btl_actors[g_btl_actor_slot].c.key][g_btl_talk_picked][line]];
            BtlTalkPickScript(g_btl_actors[g_btl_actor_slot].c.key, g_btl_talk_picked, line);
            BtlWaitBgmEnd(line);
            BtlFaceClose();
            BtlSeqStart();
            BtlSeqWaitDone();
            BtlBgmRestore();
            if ((g_btl_offer[g_btl_offer_slot].kinds
                 & (1 << (g_btl_talk_reaction & 0xFF))) != 0) {
                g_btl_panel_gauges |= 1 << (g_btl_talk_reaction & 0xFF);
                g_btl_mood_state[(g_btl_talk_reaction & 0xFF)] = 1;
                BtlPanelSetImage(1, g_btl_panel_gauges);
                BtlPushRecent((g_btl_talk_reaction & 0xFF));
            }
        } else {
            g_btl_talk_stage[g_btl_talk_depth - 1] = MENU_FACE;
            line = BtlPickLine(g_btl_talk_picked);
            BtlBgmChange(g_btl_actors[g_btl_actor_slot].c.key, g_btl_talk_picked, line);
            g_btl_talk_line_script = g_btl_talk_scripts[g_btl_talk_script_ids
                [g_btl_actors[g_btl_actor_slot].c.key][g_btl_talk_picked][line]];
            BtlTalkPickScript(g_btl_actors[g_btl_actor_slot].c.key, g_btl_talk_picked, line);
            BtlWaitBgmEnd(line);
            BtlFaceClose();
            BtlSeqStart();
            BtlSeqWaitDone();
            BtlBgmRestore();
        }
        BtlMoodRetire();
        BtlQueueVoice((g_btl_talk_reaction & 0xFF), 0);
        BtlSayDemonLine((g_btl_talk_reaction & 0xFF), (u_char)g_btl_talk_line);
        break;

    case MENU_LEAVE:
        BtlTalkersLeave();
        BtlTextOpen(g_btl_talk_left_script, TALK_TEXT_X, TALK_MEMBER_Y);
        BtlBoxOpen(TALK_BOX_COLS, TALK_BOX_X, TALK_MEMBER_Y, 0);
        BtlWaitAnyKey();
        TALK_MENU_CLOSE();
        g_btl_phase = 3;
        break;
    case MENU_WON:
        TALK_MENU_CLOSE();
        g_btl_phase = 3;
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/talkscenemenu", BtlTalkSceneMenu);
#endif
