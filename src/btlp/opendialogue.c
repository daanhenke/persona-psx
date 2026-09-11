/* Persona 1 (JP) - the lines a scripted fight opens with.  BTLP only.
 *   0x80098D38 BtlOpenDialogue   0x80099078 BtlOpeningLineWanted
 *
 * Every encounter below the last has a table of its own: eight bytes a line,
 * a sequence and whoever speaks it, ended by a null sequence. BtlStageOpen
 * runs the whole table before the first round, putting the speaker's portrait
 * up beside each line and taking it down again afterwards.
 *
 * Four encounters decide for themselves whether a line is played at all, and
 * all four decide it on the story flags - the same fight reads differently on
 * a second visit. The record is still walked and the portrait still opened;
 * only the sequence is skipped.
 *
 * Encounter 3 is the one with a scene in it: its first line is played with a
 * demon rising out of the field and going back down again around it.
 *
 * The table is walked by two pointers, the record and the speaker byte inside
 * it, because that is what the image does - one pointer and a field reference
 * gives an offset per access instead.
 *
 * 99.11%: the two walkers come out in each other's saved registers, and the
 * first of the two waits leaves its hoisted mask out of the guard's delay
 * slot where the image has it. Both are the allocator; nothing at the source
 * level moved either.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>

/* One line of an opening. */
typedef struct {
    /* 0x0 */ u_char *script;
    /* 0x4 */ u_char  who;     /* 0 for nobody, which is also no portrait */
    /* 0x5 */ u_char  pad5[3];
} BtlOpeningLine;              /* 8 bytes */

/* The tables, one pointer per encounter. */
extern BtlOpeningLine *g_btl_opening_lines[];

/* Past this there is no opening at all. */
#define OPENING_ENCOUNTERS 0x1F

/* The encounters whose lines are conditional, and the speakers each of the
   two flag tests looks at. */
#define OPENING_FLAG_ENCOUNTER_A 8
#define OPENING_FLAG_ENCOUNTER_B 0xF
#define OPENING_MOON_ENCOUNTER   0xD
#define OPENING_SCENE_ENCOUNTER  3

#define OPENING_WHO_BEFORE 5
#define OPENING_WHO_AFTER  9

/* The three flags the first test reads, one per speaker, and the one the
   moon encounter turns on. */
#define OPENING_FLAG_SPEAKER3 0x28
#define OPENING_FLAG_SPEAKER6 0x29
#define OPENING_FLAG_SPEAKER7 0x2A
#define OPENING_FLAG_MOON     0x35

/* The insert the second speaker's line carries: the first character's name. */
#define OPENING_INSERT_NAME 1
#define OPENING_INSERT_SLOT 4

/* Portraits below this belong to the party and are looked up by character. */
#define OPENING_FACE_PARTY 10
#define OPENING_FACE_X     0x3C
#define OPENING_FACE_Y     0x70
#define OPENING_FACE_SCALE 0x1000

/* Frames before the first line, between the HUD and the first line, and
   after each one. */
#define OPENING_SETTLE 60
#define OPENING_GAP    30

/* Encounter 3's demon: the fourth enemy record, the bank it plays out of and
   the two scripts that bring it up and take it away. */
#define OPENING_SCENE_BANK     0x6E
#define OPENING_SCENE_ENEMY    3
#define OPENING_SCRIPT_RISE    5
#define OPENING_SCRIPT_LEAVE   6
#define OPENING_SCENE_SLOT     3

extern void BtlFaceLoad(int who, int always);
extern void BtlFaceLoadFile(int file);
extern void BtlFaceOpen(short x, short y, short scale);
extern int  BtlEventFlagTest(int id);
extern void BtlSetInsert(int which, const u_char *src);


#ifdef NON_MATCHING
void BtlOpenDialogue(void)
{
    BtlOpeningLine *line;
    u_char         *who;
    int             i;

    if (g_btl_encounter < OPENING_ENCOUNTERS) {
        i = 0;
        do {
            i++;
            BtlDrawFrame();
        } while (i < OPENING_SETTLE);

        BtlHudLoad();
        i = 0;
        BtlHudShow();
        do {
            i++;
            BtlDrawFrame();
        } while (i < OPENING_SETTLE);

        line = g_btl_opening_lines[g_btl_encounter];
        if (line->script != NULL) {
            who = &line->who;
            do {
                if (*who != 0) {
                    /* The file case is written first: the image branches away
                       to the party one, so that is the else. */
                    if (*who >= OPENING_FACE_PARTY) {
                        BtlFaceLoadFile(*who);
                    } else {
                        BtlFaceLoad(*who, 0);
                    }
                    BtlFaceOpen(OPENING_FACE_X, OPENING_FACE_Y,
                                OPENING_FACE_SCALE);
                }
                if (*who == OPENING_INSERT_NAME) {
                    BtlSetInsert(OPENING_INSERT_SLOT, g_btl_actors[0].c.name);
                }

                switch (g_btl_encounter) {
                case OPENING_FLAG_ENCOUNTER_A:
                case OPENING_FLAG_ENCOUNTER_B:
                    if (BtlOpeningLineWanted(*who) == 0) {
                        goto skip;
                    }
                    break;
                case OPENING_MOON_ENCOUNTER:
                    /* Two arms and three branches, so this is a switch; the
                       later line is written first, which is the order the
                       bodies come out in. */
                    switch (*who) {
                    case OPENING_WHO_AFTER:
                        if (BtlEventFlagTest(OPENING_FLAG_MOON) == 0) {
                            goto skip;
                        }
                        break;
                    case OPENING_WHO_BEFORE:
                        if (BtlEventFlagTest(OPENING_FLAG_MOON) == 0) {
                            break;
                        }
                        goto skip;
                    }
                    break;
                case OPENING_SCENE_ENCOUNTER:
                    if (*who == 0) {
                        BtlHudHide();
                        BtlLoadPackBank(OPENING_SCENE_BANK);
                        BtlObjSetScript(
                            g_btl_enemies[OPENING_SCENE_ENEMY].obj,
                            g_btl_enemies[OPENING_SCENE_ENEMY].obj
                                ->scripts[OPENING_SCRIPT_RISE]);
                        while (g_btl_enemies[OPENING_SCENE_ENEMY].obj->attr
                               & BTL_OBJ_ANIMATING) {
                            BtlDrawFrame();
                        }
                        BtlHudShow();
                        i = 0;
                        do {
                            i++;
                            BtlDrawFrame();
                        } while (i < OPENING_SETTLE);

                        BtlSeqPlay(line->script);
                        BtlSeqRun();

                        BtlObjSetScript(
                            g_btl_enemies[OPENING_SCENE_ENEMY].obj,
                            g_btl_enemies[OPENING_SCENE_ENEMY].obj
                                ->scripts[OPENING_SCRIPT_LEAVE]);
                        while (g_btl_enemies[OPENING_SCENE_ENEMY].obj->attr
                               & BTL_OBJ_ANIMATING) {
                            BtlDrawFrame();
                        }
                        BtlSoundClose(OPENING_SCENE_SLOT);
                        goto skip;
                    }
                    break;
                }

                BtlSeqPlay(line->script);
                BtlSeqRun();
            skip:
                if (*who != 0) {
                    BtlFaceClose();
                }
                i = 0;
                do {
                    i++;
                    BtlDrawFrame();
                } while (i < OPENING_GAP);
                line++;
                who += sizeof(BtlOpeningLine);
            } while (line->script != NULL);
        }
        BtlHudHide();
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/opendialogue", BtlOpenDialogue);
#endif

/* Whether the speaker's line is still worth playing. Three of the four
   speakers each have a flag of their own and play only once it is set; the
   fourth is the line that stands in for all three and plays only while none
   of them is. */
int BtlOpeningLineWanted(int who)
{
    switch (who) {
    case 3:
        if (BtlEventFlagTest(OPENING_FLAG_SPEAKER3) != 0) {
            return 1;
        }
        break;
    case 6:
        if (BtlEventFlagTest(OPENING_FLAG_SPEAKER6) != 0) {
            return 1;
        }
        break;
    case 7:
        if (BtlEventFlagTest(OPENING_FLAG_SPEAKER7) != 0) {
            return 1;
        }
        break;
    case 9:
        if (BtlEventFlagTest(OPENING_FLAG_SPEAKER3) != 0) {
            break;
        }
        if (BtlEventFlagTest(OPENING_FLAG_SPEAKER6) != 0) {
            break;
        }
        if (BtlEventFlagTest(OPENING_FLAG_SPEAKER7) != 0) {
            break;
        }
        return 1;
    default:
        return 1;
    }
    return 0;
}
