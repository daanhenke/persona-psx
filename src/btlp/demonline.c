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
#include <decomp/include_asm.h>
#include <persona/btlp/battle.h>

/* Lines each reaction has. */
#define BTL_LINES 5

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

extern short   g_btl_line_cycle[];
extern short   g_btl_line_used[];
extern u_char  g_btl_scratch[];
extern u_char *g_btl_scratch_end;
extern u_char *g_btl_talk_said_script;
extern u_int   g_btl_talk_said_line;
extern u_char  g_btl_talk_nothing_script[];

extern int  VSync(int mode);
extern void BtlHighlightBegin(int who);

#ifdef NON_MATCHING
void BtlSayDemonLine(u_char act, u_char line)
{
    BtlReactionLines *ent;
    BtlReactionLines *more;
    short         *base;
    int            row;
    u_char        *script;
    u_long         dir;
    u_short        slot;
    short          fresh[8];
    int            cycle;
    int            pick;
    int            draw;
    u_short        n;
    u_short        i;
    u_short        v;

    ent = (BtlReactionLines *)(line * LINE_RECORD + LINE_FIRST + g_btl_scratch_end);
    BtlHighlightBegin(act);
    srand(VSync(-1));
    cycle = g_btl_line_cycle[act];
    if (cycle == BTL_LINE_NONE) {
        n = 0;
        v = 0;
        /* The row's base and the table's own address are both held across the
           search; recomputing either inside costs the match. */
        base = g_btl_line_used;
        row = act * BTL_LINES;
        do {
            /* A plain `while`: as a do/while gcc peels the first test, which
               the original does not. */
            i = 0;
            while (i < BTL_LINES) {
                if (base[row + i] == v) {
                    break;
                }
                i++;
            }
            if (i == BTL_LINES) {
                fresh[n] = v;
                n++;
            }
            v++;
        } while (v < BTL_LINES);
        draw = rand();
        pick = fresh[draw % n];
        i = 0;
        do {
            if (g_btl_line_used[act * BTL_LINES + i] == BTL_LINE_NONE) {
                g_btl_line_used[act * BTL_LINES + i] = fresh[draw % n];
                if (i == BTL_LINES - 1) {
                    g_btl_line_cycle[act] = 0;
                }
                break;
            }
            i++;
        } while (i < BTL_LINES);
    } else {
        pick = g_btl_line_used[act * BTL_LINES + cycle];
        g_btl_line_cycle[act] = (cycle + 1) % BTL_LINES;
    }

    if (pick < LINE_DIRECT) {
        slot = ent[act].slot[pick];
    } else {
        more = (BtlReactionLines *)(g_btl_scratch_end + LINE_MORE
                                 + (ent[act].more & LINE_MORE_MASK)
                                   * LINE_RECORD);
        slot = more[act].slot[pick - LINE_DIRECT];
    }
    dir = *(u_long *)g_btl_scratch;
    script = g_btl_scratch + dir + *(u_long *)(g_btl_scratch + dir + slot * 4);
    g_btl_talk_said_script = script;
    g_btl_talk_said_line = slot;

    BtlDrawFrame();
    BtlDrawFrame();
    BtlDrawFrame();

    if ((u_long)(script - g_btl_scratch) >= BTL_SCRATCH_SIZE) {
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
#else
INCLUDE_ASM("btlp/nonmatchings/demonline", BtlSayDemonLine);
#endif

