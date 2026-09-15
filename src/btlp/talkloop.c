/* Persona 1 (JP) - opening the talk, and how a demon takes an act.  BTLP only.
 *   0x8006B6B0 BtlTalkLoop
 *   0x8006B968 BtlPickReaction
 *
 * BtlTalkLoop is where the battle asks whether a talk can start at all. An
 * offer the party has already taken the Persona from ends it on the spot. A
 * party that turned the demons down before is told so, and one whose members
 * none of them will talk to is told that; otherwise the talk's menu is pushed,
 * the markers are pulled back and the music is let down under it. Whichever
 * way it goes, a talk that did not open leaves the scene stack on its idle
 * scene.
 *
 * BtlPickReaction rolls how the demon takes the act a member picked. The
 * member, the act and whether the offer is wary pick a row of five entries in
 * g_btl_reactions; the first entry whose condition holds - a bit of the
 * offer's flags set, or from nine up a bit clear, or no condition at all -
 * gives four odds out of a hundred, one per gauge. An offer that shifts its
 * odds has them moved by the first row of g_btl_reaction_shifts that leaves the
 * first gauge the entry does not reach alone, clamped to a hundred and, for the
 * first gauge, kept above nothing where it was above nothing. A roll of the
 * hundred then walks the odds and answers the gauge it lands on.
 */
#include <decomp/types.h>
#include <memory.h>
#include <rand.h>
#include <libsnd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/box.h>
#include <persona/btlp/input.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/panel.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/talk.h>
#include <persona/btlp/text.h>

/* The scenes BtlTalkLoop leaves on the stack. */
#define TALK_SCENE_IDLE 0
#define TALK_SCENE_MENU 10

/* Bit 0 of g_btl_talk_flags: the party asked for the talk itself. */
#define TALK_FLAG_ASKED 1

/* Where the boxes go. */
#define TALK_LOOP_TEXT_X 0x28
#define TALK_LOOP_Y      0x9C
#define TALK_LOOP_COLS   0x11
#define TALK_LOOP_BOX_X  0xA0

/* How far the music is let down, and over how many frames. */
#define TALK_BGM_DIP    0x57
#define TALK_BGM_FRAMES 0x3C

/* BtlOffer.flags: the offer is wary of the party, and its odds are shifted. */
#define OFFER_WARY    0x100
#define OFFER_SHIFTED 0x4000

/* The member whose odds are never shifted. */
#define REACTION_UNSHIFTED_KEY 9

#define REACTION_ENTRIES 5
#define REACTION_SHIFTS  5

int BtlTalkLoop(void)
{
    int i;
    int opened;

    opened = 0;
    BtlDrawFrame();
    BtlInputClear();
    for (i = 0; i < BTL_OFFERS; i++) {
        if ((g_btl_offer[i].flags & OFFER_TAKEN) != 0) {
            BtlTextOpen(g_btl_talk_unpickable_script, TALK_LOOP_TEXT_X, TALK_LOOP_Y);
            BtlBoxOpen(TALK_LOOP_COLS, TALK_LOOP_BOX_X, TALK_LOOP_Y, 0);
            BtlWaitAnyKey();
            BtlShowAilmentMarks(1);
            BtlFaceClose();
            BtlPanelClose();
            BtlBoxClose();
            BtlSeqClear();
            BtlHudHide();
            BtlEnemiesReset();
            BtlPartyReset();
            g_btl_phase = 0;
            return 0;
        }
    }
    if (g_btl_talk_flags != 0) {
        if ((g_btl_talk_flags & TALK_FLAG_ASKED) != 0) {
            goto open;
        }
        BtlTextOpen(g_btl_talk_turned_down_script, TALK_LOOP_TEXT_X, TALK_LOOP_Y);
        BtlBoxOpen(TALK_LOOP_COLS, TALK_LOOP_BOX_X, TALK_LOOP_Y, 0);
        BtlWaitAnyKey();
        BtlShowAilmentMarks(1);
        BtlFaceClose();
        BtlPanelClose();
        BtlBoxClose();
        BtlSeqClear();
        BtlHudHide();
        BtlEnemiesReset();
        BtlPartyReset();
        g_btl_phase = 0;
    } else {
        for (i = 0; i < BTL_OFFERS; i++) {
            if (g_btl_offer[i].used != 0
                && (g_btl_offer[i].kinds & OFFER_CONTACTED) == 0) {
                break;
            }
        }
        if (g_btl_member_matched != 0) {
        open:
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_MENU;
            g_btl_talk_stage[g_btl_talk_depth] = 1;
            g_btl_talk_depth++;
            opened = 1;
            BtlPickSettle();
            BtlRetractMarkers();
            SsSepSetDecrescendo(g_btl_seq[0], 0, BtlSeqVolumeMean() - TALK_BGM_DIP,
                                TALK_BGM_FRAMES);
        } else {
            BtlTextOpen(g_btl_talk_mute_script, TALK_LOOP_TEXT_X, TALK_LOOP_Y);
            BtlBoxOpen(TALK_LOOP_COLS, TALK_LOOP_BOX_X, TALK_LOOP_Y, 0);
            BtlWaitAnyKey();
            BtlShowAilmentMarks(1);
            BtlFaceClose();
            BtlPanelClose();
            BtlBoxClose();
            BtlSeqClear();
            BtlHudHide();
            BtlEnemiesReset();
            BtlPartyReset();
            g_btl_phase = 0;
        }
    }
    if (opened == 0) {
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_IDLE;
        g_btl_talk_stage[g_btl_talk_depth] = 1;
        g_btl_talk_depth++;
    }
}

int BtlPickReaction(int picked)
{
    u_short              odds[BTL_MOODS];
    const u_char        *e;
    const short         *shift;
    int                  wary;
    int                  i;
    int                  j;
    int                  k;
    int                  v;
    int                  floor;
    int                  ge;

    wary = 0;
    if ((g_btl_offer[g_btl_offer_slot].flags & OFFER_WARY) != 0) {
        wary = 1;
    }
    bzero((u_char *)odds, sizeof(odds));
    e = (const u_char *)&((const BtlReaction *)g_btl_reactions)
        [(g_btl_actors[g_btl_actor_slot].c.key - 1) * 40 + picked * 10 + wary * 5];
    for (i = 0; i < REACTION_ENTRIES; i++, e += sizeof(BtlReaction)) {
        if (e[0] == 0 || i == REACTION_ENTRIES - 1) {
            break;
        }
        ge = e[0] >= 9;
        if (!ge) {
            if ((g_btl_offer[g_btl_offer_slot].flags >> (e[0] - 1) & 1) != 0) {
                break;
            }
        } else {
            if ((g_btl_offer[g_btl_offer_slot].flags >> (e[0] - 9) & 1) == 0) {
                break;
            }
        }
    }
    if (i == REACTION_ENTRIES) {
        i = 0;
    }
    /* The entry is walked a byte at a time; a pointer to BtlReaction and
       its odds[i] give the copy a second register. */
    e = (const u_char *)&((const BtlReaction *)g_btl_reactions)
        [(g_btl_actors[g_btl_actor_slot].c.key - 1) * 40 + picked * 10 + wary * 5];
    e += i * sizeof(BtlReaction) + 1;
    for (i = 0; i < BTL_MOODS; i++) {
        odds[i] = *e++;
    }

    if ((g_btl_offer[g_btl_offer_slot].flags & OFFER_SHIFTED) != 0
        && g_btl_actors[g_btl_actor_slot].c.key != REACTION_UNSHIFTED_KEY) {
        for (i = 0; i < BTL_MOODS; i++) {
            if (odds[i] == 0) {
                break;
            }
        }
        if (i == BTL_MOODS) {
            k = 0;
        } else {
            for (k = 0; k < REACTION_SHIFTS; k++) {
                if (g_btl_reaction_shifts[k][i] == 0) {
                    break;
                }
            }
        }
        for (j = 0; j < BTL_MOODS; j++) {
            floor = 0;
            if (j == 0) {
                floor = odds[0] != 0;
            }
            v = odds[j] + g_btl_reaction_shifts[k][j];
            if (v > 100) {
                v = 100;
            }
            if (v < floor) {
                v = floor;
            }
            odds[j] = v;
        }
    }

    v = rand() % 100 + 1;
    k = 0;
    for (i = 0; i < BTL_MOODS; i++) {
        k += odds[i];
        if (k >= v) {
            break;
        }
    }
    if (i == BTL_MOODS) {
        i = 0;
    }
    return (u_short)i;
}
