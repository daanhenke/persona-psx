/* Persona 1 (JP) - deciding whether a negotiation starts, and starting it.
 *   0x800686AC BtlBeginTalking    BTLP only.
 *
 * The counterpart to BtlEndTalking, and the longest single stretch of the
 * negotiation. It answers whether one is under way.
 *
 * Everything is a reason not to, in order: nobody is acting, the acting
 * member is not the kind who can talk, the moon is neither new nor full, a
 * roll of four does not come up zero, no offer is still live, or the party
 * already holds the Persona the drawn offer would hand over. Offers that are
 * spoken for, or that are not being shown, are struck out of the live set
 * before the draw rather than skipped during it.
 *
 * Once it is on, the demons' voices are read in - one 24000-byte slot each
 * for every member still fighting and for the odd-numbered sound slots - and
 * the moon decides which set of opening lines is drawn from. A one-in-three
 * roll overrides both moons with a third set of its own. The line that comes
 * out carries a mask of which mood gauges it flatters: those are raised by
 * fifty and capped, and the rest are halved and floored at one.
 *
 * The player still gets the last word. The box asks, and answering one puts
 * everything away again and leaves the battle where it was.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/member.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/panel.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/status.h>
#include <persona/btlp/talk.h>
#include <persona/btlp/text.h>
#include <persona/common/persona.h>

/* Which moons let a contact be opened at all. */
#define MOON_NEW  0
#define MOON_FULL 8

/* The three sets of opening lines: the full moon's, the new moon's, and the
   one a roll of three reaches past both. */
#define TALK_OPEN_SETS  3
#define TALK_OPEN_SPARE 2

/* What a flattered gauge gains, and the ceiling it stops at. Everything else
   is halved, and never falls below one. */
#define TALK_OPEN_GAIN 0x32
#define TALK_OPEN_CAP  0x46

/* Where the demons' voices are read to, and how much room each one takes. */
#define TALK_VOICE_BUFFER ((u_long *)0x80172400)
#define TALK_VOICE_BYTES  0x5DC0

/* The sound slots the odd-numbered banks live in. */
#define TALK_SLOT_FIRST 10
#define TALK_SLOT_LAST  15
#define TALK_SLOT_STEP  2

/* What the box answers, and the flag a refusal leaves behind. */
#define TALK_ANSWER_NO   1
#define TALK_TURNED_DOWN 2

/* Where the opening line's own scripts sit in the scratch file, and the three
   the roll picks between. */
#define TALK_OPEN_TABLE 0x364
#define TALK_OPEN_LINES 3

/* The two scenes pushed as the negotiation opens, and the stage the first is
   corrected to once the second is on top of it. */
#define TALK_SCENE_OPEN  10
#define TALK_SCENE_FIRST 12
#define TALK_STAGE_OPEN  1
#define TALK_STAGE_HELD  9

/* The insert slots the opening message fills. */
#define TALK_INSERT_MEMBER 4
#define TALK_INSERT_DEMON  2
#define TALK_INSERT_ARCANA 6

/* Where the question is put, and the sound slot the answer closes. */
#define TALK_BOX_COLS  0x11
#define TALK_BOX_X     0xA0
#define TALK_BOX_Y     0x92
#define TALK_TEXT_X    0x28
#define TALK_TEXT_Y    0x92
#define TALK_ASK_SLOT  3
#define TALK_OPEN_WAIT 0xF

/* Marks the offer as the one being talked to. */
#define OFFER_TALKING 0x8000000

/* The scratch buffer, reached by address rather than through the linker's
   symbol: its low half is zero, so one lui is the whole of it. */

/* Declared here rather than taken from persona/btlp/battle.h: hud.c
   defines it int, and the byte the call sites want is what makes the
   andi come out. The two forms are not interchangeable. */
extern char BtlHudState(void);
extern u_long       *g_btl_scratch_end;
extern const u_char *g_btl_arcana_names[];
extern volatile long g_cd_busy;

extern const u_char *g_btl_talk_open_text;
extern const u_char *g_btl_talk_open_menu[];
extern const u_char  g_btl_talk_open_counts[];
extern const u_char  g_btl_talk_open_moods[];
extern const u_char  g_btl_talk_open_lines[];
extern int           g_btl_talk_sounds_loaded;

extern int   BtlStockHolds(const BtlOffer *offer);
extern short BtlPickTalkTarget(short mask);
extern void  BtlTintTalkers(void);
extern void  BtlTintParty(void);
extern void  BtlSeekFile(int index);
extern void  BtlBoxOpen(short cols, short x, short y, int style);
extern void  BtlHudShow(void);
extern void  BtlLoadScratch(int index, int from_table);
extern void  BtlMenuOpen2(const u_char **text);
extern void  BtlIndicatorBar(void);
extern int   BtlMenuChoice(void);
extern void  BtlMenuUpdate(void);
extern void  BtlTalkUpdate(void);
extern void  BtlSoundClose(int slot);
extern void  BtlMenuHide(void);
extern void  BtlSetInsert(int which, const u_char *src);
extern void  BtlFaceLoad(int who, int always);
extern void  BtlRefreshMoodGauges(void);
extern void  BtlMenuDismiss(void);
extern int   BtlMenuState(void);

#ifdef NON_MATCHING
#ifdef NON_MATCHING
int BtlBeginTalking(void)
{
    const BtlMember *m;
    short           *gauge;
    u_long          *dest;
    u_char          *p;
    u_short         *line;
    u_char           live[8];
    int              n;
    int              other;
    int              set;
    int              pick;
    int              answer;
    int              slot;
    int              value;
    int              i;
    u_long           dir;

    m = g_btl_member;
    for (i = 0; i < BTL_PARTY; i++) {
        if (m->key == 1) {
            g_btl_actor_slot = i;
            break;
        }
        m++;
    }
    if (i == BTL_PARTY) {
        return 0;
    }

    i = 0;
    do {
        if ((g_btl_offer[i].flags & OFFER_TAKEN) != 0
            || (g_btl_offer[i].flags & OFFER_SCORED) == 0) {
            g_btl_offer_live &= ~(1 << i);
        }
        i++;
    } while (i < BTL_OFFERS);

    if (m->answer == 1) {
        if (g_btl_moon != MOON_NEW && g_btl_moon != MOON_FULL) {
            return 0;
        }
        if ((rand() & 3) != 0) {
            return 0;
        }
        if (g_btl_offer_live != 0) {

            n = 0;
            i = 0;
            p = live;
            do {
                if ((g_btl_offer_live >> i & 1) != 0) {
                    *p = i;
                    p++;
                    n++;
                }
                i++;
            } while (i < BTL_OFFERS);
            if (n == 0) {
                return 0;
            }

            slot = live[rand() % n];
            g_btl_offer_slot = slot;
            if (BtlStockHolds(&g_btl_offer[slot]) != 0) {
                return 0;
            }

            dest = TALK_VOICE_BUFFER;
            i = 0;
            do {
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                    BtlLoadSound(dest, g_btl_actors[i].c.key);
                    dest = (u_long *)((int)dest + TALK_VOICE_BYTES);
                }
                i++;
            } while (i < BTL_PARTY);

            i = TALK_SLOT_FIRST;
            do {
                if (g_btl_slot_owner[i] >= 0) {
                    BtlLoadSlotSound(dest, i);
                    dest = (u_long *)((int)dest + TALK_VOICE_BYTES);
                }
                i += TALK_SLOT_STEP;
            } while (i < TALK_SLOT_LAST);

            g_btl_talk_sounds_loaded = 1;

            gauge = g_btl_offer[g_btl_offer_slot].mood;
            other = g_btl_moon != MOON_FULL;
            set = other;
            if (rand() % TALK_OPEN_SETS == 0) {
                set = TALK_OPEN_SPARE;
            }
            i = 0;
            pick = rand() % g_btl_talk_open_counts[set] + set * TALK_OPEN_SETS;
            do {
                if ((g_btl_talk_open_moods[pick] >> i & 1) != 0) {
                    value = *gauge + TALK_OPEN_GAIN;
                    if (value > TALK_OPEN_CAP) {
                        value = TALK_OPEN_CAP;
                    }
                } else {
                    value = *gauge / 2;
                    if (value <= 0) {
                        value = 1;
                    }
                }
                *gauge = value;
                i++;
                gauge++;
            } while (i < BTL_MOODS);

            g_btl_talk_target = BtlPickTalkTarget(g_btl_offer[g_btl_offer_slot].used);
            BtlTintTalkers();
            BtlTintParty();
            BtlLoadPackBank(g_btl_offer[g_btl_offer_slot].persona);
            BtlSeekFile(g_btl_offer[g_btl_offer_slot].voice);
            BtlTextOpen(g_btl_talk_open_text, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_COLS, TALK_BOX_X, TALK_BOX_Y, 0);
            BtlHudLoad();
            BtlHudShow();
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

            BtlMenuOpen2(g_btl_talk_open_menu);
            BtlIndicatorBar();
            BtlMenuUpdate();
            BtlTalkUpdate();
            while ((answer = BtlMenuChoice()) < 0) {
                BtlMenuUpdate();
                BtlTalkUpdate();
                BtlDrawFrame();
            }

            g_btl_battle_kind = 0;
            if (answer == TALK_ANSWER_NO) {
                g_btl_talk_flags |= TALK_TURNED_DOWN;
                BtlSoundClose(TALK_ASK_SLOT);
                BtlMenuHide();
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

            BtlSetInsert(TALK_INSERT_MEMBER, g_btl_actors[g_btl_actor_slot].c.name);
            BtlSetInsert(TALK_INSERT_DEMON, g_btl_offer[g_btl_offer_slot].name);
            BtlSetInsert(TALK_INSERT_ARCANA,
                         g_btl_arcana_names[g_persona_data[
                             g_btl_offer[g_btl_offer_slot].persona].arcana]);
            BtlFaceLoad(g_btl_actors[g_btl_actor_slot].c.key, 0);
            g_btl_offer[g_btl_offer_slot].kinds |= OFFER_TALKING;
            BtlBoxClose();
            BtlIndicatorClear();
            BtlRefreshMoodGauges();
            BtlPanelLoad();
            BtlPanelOpen();

            /* The line's own three scripts are found before the roll that picks one,
               so the roll lands in the call's delay slot rather than ahead of it. */
            value = (int)g_btl_scratch_end + TALK_OPEN_TABLE;
            line = (u_short *)(value + g_btl_talk_open_lines[pick] * 8);
            line += rand() % TALK_OPEN_LINES;
            BtlRunFrames(TALK_OPEN_WAIT);
            BtlMenuDismiss();
            while (BtlMenuState() != 0) {
                BtlMenuUpdate();
                BtlTalkUpdate();
                BtlDrawFrame();
            }

            dir = *(u_long *)BTL_SCRATCH;
            BtlSeqPlay(BTL_SCRATCH + dir
                       + *(u_long *)(BTL_SCRATCH + dir + *line * 4));

            g_btl_phase = 1;
            g_btl_talking = 1;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_OPEN;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_OPEN;
            g_btl_talk_depth++;
            g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_HELD;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_FIRST;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_OPEN;
            g_btl_talk_depth++;
            BtlTalkSceneStep();
            return 1;
        }
    }
    return 0;
}
#else
INCLUDE_ASM("btlp/nonmatchings/begintalk", BtlBeginTalking);
#endif
#else
INCLUDE_ASM("btlp/nonmatchings/begintalk", BtlBeginTalking);
#endif
