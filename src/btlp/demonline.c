/* Persona 1 (JP) - what the demons say back, and saying it.  BTLP only.
 *   0x8006830C BtlSayDemonLine
 *
 * There are five lines for each of the four reactions BtlPickReaction chooses
 * between, and they are dealt out the way BtlPickLine deals the party's three:
 * while that reaction's cycle is -1 the lines absent from its row of
 * g_btl_line_used are collected, one is taken at random and written into the
 * first free place, and filling the last place starts the cycle at zero. After
 * that the cycle steps round the five in the order they were dealt.
 *
 * The line is then looked up in the scratch pack. One 0x18-byte record per
 * talk line sits at the end of what was loaded, six bytes of it per reaction:
 * two directory slots for lines 0 and 1, and a third short naming a record in
 * a second table further on that the last three lines come from. The slot
 * indexes the pack's directory exactly as BtlMessage does.
 *
 * A line the pack does not have - slot 0xFFFF, or a script that resolves
 * outside the buffer - is replaced by the one that says nothing. Either way
 * the talk scene that waits for the sequencer is pushed on top.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

/* Nothing dealt yet. */
#define BTL_LINE_NONE (-1)

/* The per-line record at the end of the pack, and the table the last three
   lines are read out of. */
#define LINE_RECORD    0x18
#define LINE_FIRST     4
#define LINE_MORE      0x478
#define LINE_MORE_MASK 0x7FFF

/* Lines 0 and 1 come from the record itself; 2 and up from the second table. */
#define LINE_DIRECT 2

/* The scratch buffer is 64K, and a script resolving past it is not one. */
#define BTL_SCRATCH_SIZE 0x10000

/* No directory slot. */
#define BTL_SLOT_NONE 0xFFFF

/* The talk scene that waits for the sequencer. */
#define TALK_SCENE_WAIT 0xC
#define TALK_STAGE_RUN  1

/* One reaction's share of a line record. */
typedef struct {
    /* 0x0 */ u_short slot[LINE_DIRECT]; /* directory slots for lines 0 and 1 */
    /* 0x4 */ u_short more;              /* which record of the second table
                                            the other three come from; the top
                                            bit is not part of the number    */
} BtlReactionLines;                         /* 6 bytes */

extern u_char *g_btl_talk_said_script;
extern u_int   g_btl_talk_said_line;
extern u_char  g_btl_talk_nothing_script[];

extern int  VSync(int mode);
extern void BtlHighlightBegin(int who);

/* One variable walks the candidates and then holds the line that was picked,
   whichever way it was picked - the image keeps both in the same register.
   The line table is two-dimensional, a row per demon, which is what makes the
   row's address come out once per candidate. */
void BtlSayDemonLine(u_char act, u_char line)
{
    BtlReactionLines *ent;
    BtlReactionLines *more;
    u_char           *script;
    u_char           *at;
    u_long            dir;
    u_short           fresh[BTL_DEMON_LINES];
    u_short           slot;
    u_short           n;
    u_short           pick;
    u_short           i;
    int               cycle;

    ent = (BtlReactionLines *)(line * LINE_RECORD + LINE_FIRST + (u_char *)g_btl_scratch_end);
    BtlHighlightBegin(act);
    srand(VSync(-1));
    cycle = g_btl_line_cycle[act];
    if (cycle == BTL_LINE_NONE) {
        n = 0;
        for (pick = 0; pick < BTL_DEMON_LINES; pick++) {
            for (i = 0; i < BTL_DEMON_LINES; i++) {
                if (g_btl_line_used[act][i] == pick) {
                    break;
                }
            }
            if (i == BTL_DEMON_LINES) {
                fresh[n] = pick;
                n++;
            }
        }
        pick = fresh[rand() % n];
        for (i = 0; i < BTL_DEMON_LINES; i++) {
            if (g_btl_line_used[act][i] == BTL_LINE_NONE) {
                g_btl_line_used[act][i] = pick;
                if (i == BTL_DEMON_LINES - 1) {
                    g_btl_line_cycle[act] = 0;
                }
                break;
            }
        }
    } else {
        pick = g_btl_line_used[act][cycle];
        g_btl_line_cycle[act] = (cycle + 1) % BTL_DEMON_LINES;
    }

    if (pick < LINE_DIRECT) {
        /* Through a pointer to the record's shorts: as ent[act].slot[pick]
           gcc works out the record's address before the index's. */
        slot = ((u_short *)&ent[act])[pick];
        dir = *(u_long *)BTL_SCRATCH;
        at = *(u_long *)(BTL_SCRATCH + dir + slot * 4) + BTL_SCRATCH;
    } else {
        more = (BtlReactionLines *)((u_char *)g_btl_scratch_end
                                    + (ent[act].more & LINE_MORE_MASK)
                                      * LINE_RECORD
                                    + LINE_MORE);
        dir = *(u_long *)BTL_SCRATCH;
        at = *(u_long *)(BTL_SCRATCH + dir
                         + more[act].slot[pick - LINE_DIRECT] * 4)
             + BTL_SCRATCH;
        slot = more[act].slot[pick - LINE_DIRECT];
    }
    script = at + dir;
    g_btl_talk_said_script = script;
    g_btl_talk_said_line = slot;

    BtlDrawFrame();
    BtlDrawFrame();
    BtlDrawFrame();

    if ((u_long)(script - BTL_SCRATCH) >= BTL_SCRATCH_SIZE) {
        script = g_btl_talk_nothing_script;
    }
    if (slot == BTL_SLOT_NONE) {
        script = g_btl_talk_nothing_script;
    }
    BtlSeqPlay(script);
    g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_WAIT;
    g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
    g_btl_talk_depth++;
}

