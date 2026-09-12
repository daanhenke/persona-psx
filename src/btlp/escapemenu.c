/* Persona 1 (JP) - running away.  BTLP only.
 *   0x8009EEB4 BtlEscapeMenu
 *
 * Entry 3 of g_btl_pick_command. Like the other commands that take the screen
 * for themselves it runs its own frame loop, stepping through g_btl_step
 * rather than being ticked from outside, and only comes back when the party
 * has either got away or been caught.
 *
 * Whether it works is settled in two places. Step nought rules the roll out
 * altogether - a fight that may not be left, a field that says so, an enemy
 * ten levels or more above the party, or nobody on either side who can be got
 * past - and when it does not, it turns the party's edge in agility and luck
 * into a starting chance out of 0x100. Step one then bends that chance by how
 * long the player held the key down: the chance picks a row, the number of
 * presses picks a column, and the table at 0x800CFCD0 says what to add.
 *
 * Everything from there is the two endings. Getting away puts every member on
 * the leaving motion, waits for the markers to come in, and hands back with
 * the stage set to two; being caught marks the party and goes on with the
 * fight as a second round.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sides.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/status.h>
#include <persona/btlp/text.h>

/* Stride of g_btl_member_scripts, by Char key and by the actor's
   script_pick, and the entry of the ten a member leaves on. */
#define MEMBER_SCRIPT_MODEL 0x28
#define MEMBER_SCRIPT_PICK  10
#define SCRIPT_LEAVE        0

/* How the three lines are put up, and the three of them: the question, the
   answer when the party gets away, and the answer when it does not. */
#define ESCAPE_MSG_WIDTH 0x10
#define ESCAPE_MSG_STYLE 0x94
extern u_char D_800CFA2C;
extern u_char D_800CFA50;
extern u_char D_800CFA58;

/* Frames the question stands for, and frames the failure stands for. */
#define ESCAPE_WAIT 0xF0
#define ESCAPE_HELD 0x3C

/* Three ways the roll is ruled out before it is made: the fight is one that
   may not be left, the field says so, and the level gap. */
extern u_char D_8004E260;
extern u_char D_800CCA2E;
extern u_char D_800CCA34;

/* How many times the key was pressed while the question stood, kept where
   something else can read it. */
extern u_char D_800CCA35;

/* The chance the roll starts from, by how far the party's agility and luck
   are ahead: the first row whose edge the party reaches gives the chance.
   Seven of each, and a byte of padding between the two. */
extern signed char D_800CFCAC[];
extern u_char      D_800CFCB4[];
#define ESCAPE_EDGES 7

/* What the number of presses is worth. The chance picks one of nine rows and
   the presses one of seven columns; the table is eight wide. */
extern u_char D_800CFCBC[];
extern u_char D_800CFCC8[];
extern u_char D_800CFCD0[];
#define ESCAPE_ROWS 9
#define ESCAPE_COLS 7
#define ESCAPE_ROW_BYTES 8

/* The chance is rolled out of a byte and pinned to the range. */
#define ESCAPE_CHANCE_MAX 0x100
#define ESCAPE_ROLL_MASK  0xFF

/* The fight the party is caught into, and the stage a fight left behind
   goes to. */
#define ESCAPE_KIND_NO_LEAVE 1
#define ESCAPE_KIND_CAUGHT   2
#define ESCAPE_STAGE_LEFT    2

/* Set on a member that is being carried through the escape, and the one that
   says the member is on its way out. */
#define ESCAPE_CAUGHT_BIT 0x20000
#define ESCAPE_LEAVE_BIT  0x80000000
#define ESCAPE_SPARED     0x4000

/* The motion a member leaves on, the frames the first of them waits, and how
   much later each one behind it goes. */
#define ESCAPE_MOTION 0xE
#define ESCAPE_DELAY  0x1E
#define ESCAPE_STAGGER 0x10

/* Cleared off the leading record once they are all away. */
#define ESCAPE_HIDDEN 0x40000000

/* Enemies the walk looks at. */
#define ESCAPE_ENEMIES 9

#ifdef NON_MATCHING
int BtlEscapeMenu(void)
{
    const u_char *line;
    int           chance;
    int           got;
    int           presses;
    int           row;
    int           col;
    int           edge;
    int           i;
    short         timer;
    int           done;
    /* The two the whole loop keeps in a register: the frames a question
       stands for, and the bit a member on its way out carries. */
    short         wait;
    u_long        leaving;

    wait = ESCAPE_WAIT;
    leaving = ESCAPE_LEAVE_BIT;
    do {
        switch (g_btl_step) {
        case 0:
            got = 0;
            BtlOpenMessage(0, 0, &D_800CFA2C, ESCAPE_MSG_WIDTH,
                           ESCAPE_MSG_STYLE);
            if (g_btl_battle_kind == ESCAPE_KIND_NO_LEAVE || D_8004E260 != 0) {
                got = 1;
            } else if (D_800CCA2E != 0
                       || g_btl_enemy_level - g_btl_party_level >= 10
                       || D_800CCA34 != 0) {
                got = 0;
            } else {
                i = 0;
                do {
                    if (g_btl_actors[i].c.key != 0
                        && *(signed char *)&g_btl_actors[i].c.status
                               != BTL_STATUS_DOWN
                        && (g_btl_actors[i].flags & ESCAPE_SPARED) == 0
                        && BtlStatusStops(&g_btl_actors[i]) != 0) {
                        break;
                    }
                    i++;
                } while (i < BTL_PARTY);
                if (i < BTL_PARTY) {
                    i = 0;
                    do {
                        if (g_btl_combatants[i].c.key != 0
                            && BtlStatusStops(&g_btl_combatants[i]) != 0) {
                            break;
                        }
                        i++;
                    } while (i < ESCAPE_ENEMIES);
                    if (i < ESCAPE_ENEMIES) {
                        chance = 0;
                        edge = (g_btl_party_agility + g_btl_party_luck) / 2
                               - (g_btl_enemy_agility + g_btl_enemy_luck) / 2;
                        i = 0;
                        do {
                            if (edge >= D_800CFCAC[i]) {
                                chance = D_800CFCB4[i];
                                break;
                            }
                            i++;
                        } while (i < ESCAPE_EDGES);
                        g_btl_delay = wait;
                        presses = 0;
                        g_btl_step++;
                        break;
                    }
                    got = 1;
                } else {
                    got = 0;
                }
            }
            g_btl_delay = wait;
            g_btl_step = 2;
            break;
        case 1:
            if (g_btl_delay != 0) {
                if ((g_btl_pad1_edge & g_btl_key_confirm) != 0) {
                    presses++;
                    D_800CCA35 = presses;
                }
                break;
            }
            row = ESCAPE_ROWS;
            i = 0;
            do {
                if (D_800CFCBC[i] >= chance) {
                    row = i;
                    break;
                }
                i++;
            } while (i < ESCAPE_ROWS);
            col = ESCAPE_COLS;
            i = 0;
            do {
                if (D_800CFCC8[i] >= presses) {
                    col = i;
                    break;
                }
                i++;
            } while (i < ESCAPE_COLS);
            chance += D_800CFCD0[row * ESCAPE_ROW_BYTES + col];
            if (chance < 0) {
                chance = 0;
            } else if (chance > ESCAPE_CHANCE_MAX) {
                chance = ESCAPE_CHANCE_MAX;
            }
            if ((int)(rand() & ESCAPE_ROLL_MASK) < chance) {
                got = 1;
            }
            g_btl_step++;
            break;
        case 2:
            if (g_btl_delay != 0) {
                break;
            }
            line = &D_800CFA58;
            if (got != 0) {
                line = &D_800CFA50;
            }
            BtlTextSetState(5, 0, 1);
            BtlTextWaitDone();
            i = 1;
            do {
                i++;
                BtlDrawFrame();
            } while (i != 7);
            BtlOpenMessage(0, 0, line, ESCAPE_MSG_WIDTH, ESCAPE_MSG_STYLE);
            if (got != 0) {
                g_btl_step += 2;
            } else {
                g_btl_delay = ESCAPE_HELD;
                g_btl_step++;
            }
            break;
        case 3:
            if (g_btl_delay != 0) {
                break;
            }
            i = 0;
            do {
                if (g_btl_actors[i].c.key != 0
                    && *(signed char *)&g_btl_actors[i].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & ESCAPE_SPARED) == 0) {
                    g_btl_actors[i].flags |= ESCAPE_CAUGHT_BIT;
                }
                i++;
            } while (i < BTL_PARTY);
            BtlPickSettle();
            BtlPartyResetGfx();
            BtlCloseMessage(0);
            g_btl_battle_kind = ESCAPE_KIND_CAUGHT;
            return 1;
        case 4:
            BtlHideMarkers();
            i = 0;
            timer = ESCAPE_DELAY;
            do {
                if (g_btl_actors[i].c.key != 0
                    && *(signed char *)&g_btl_actors[i].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & ESCAPE_SPARED) == 0) {
                    BtlObjSetScript(
                        g_btl_actors[i].obj,
                        (BtlSeqStep *)g_btl_actors[i].obj->scripts[
                            g_btl_member_scripts[
                                g_btl_actors[i].c.key * MEMBER_SCRIPT_MODEL
                                + SCRIPT_LEAVE
                                + g_btl_actors[i].script_pick
                                      * MEMBER_SCRIPT_PICK]]);
                    g_btl_actors[i].obj->motion = ESCAPE_MOTION;
                    g_btl_actors[i].obj->timer = timer;
                    timer += ESCAPE_STAGGER;
                    g_btl_actors[i].flags |= leaving;
                }
                i++;
            } while (i < BTL_PARTY);
            g_btl_step++;
            break;
        case 5:
            done = 1;
            i = 0;
            do {
                if (g_btl_actors[i].c.key != 0
                    && (g_btl_actors[i].flags & leaving) != 0
                    && g_btl_actors[i].obj->motion != 0) {
                    done = 0;
                    break;
                }
                i++;
            } while (i < BTL_PARTY);
            if (done == 0) {
                break;
            }
            i = 0;
            do {
                if (g_btl_actors[i].c.key != 0
                    && (g_btl_actors[i].flags & leaving) != 0) {
                    g_btl_actors[i].obj->attr &= ~ESCAPE_HIDDEN;
                }
                i++;
            } while (i < BTL_PARTY);
            do {
                BtlDrawFrame();
            } while (BtlMarkersHidden() == 0);
            g_btl_stage = ESCAPE_STAGE_LEFT;
            return 1;
        }
        BtlDrawFrame();
    } while (1);
}
#else
INCLUDE_ASM("btlp/nonmatchings/escapemenu", BtlEscapeMenu);
#endif
