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

/* The chance the roll starts from, by how far the party's agility and luck
   are ahead: the first row whose edge the party reaches gives the chance.
   Seven of each, and a byte of padding between the two. */
extern signed char D_800CFCAC[];
extern u_char      D_800CFCB4[];
#define ESCAPE_EDGES 7

/* What the number of presses is worth. The chance picks one of nine rows and
   the presses one of seven columns; the table is eight wide. */
#define ESCAPE_ROWS 9
#define ESCAPE_COLS 7
#define ESCAPE_ROW_BYTES 8
extern u_char D_800CFCBC[];
extern u_char D_800CFCC8[];
extern u_char D_800CFCD0[][ESCAPE_ROW_BYTES];

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

/* Every exit of step 0 writes its own "wait, then step two" tail; the image
   has them folded together by cross-jumping after reload, but the copies are
   what make the wait worth a saved register. The leaving bit is a literal
   that loop.c lifts into one. The walks' counter is shared by every step but
   the second, whose frame count and the other steps' scratch values share a
   variable of their own. */
int BtlEscapeMenu(void)
{
    const u_char *line;
    int           chance;
    int           got;
    int           presses;
    int           rank;
    int           col;
    int           i;
    int           n;
    /* Added to the chance before the presses are weighed, and never
       anything but nought. */
    int           bonus;
    u_char       *row;
    /* The frames a question stands for, kept in a register all loop long. */
    short         wait;

    wait = ESCAPE_WAIT;
    bonus = 0;
    do {
        switch (g_btl_step) {
        case 0:
            got = 0;
            BtlOpenMessage(0, 0, &D_800CFA2C, ESCAPE_MSG_WIDTH,
                           ESCAPE_MSG_STYLE);
            if (g_btl_battle_kind == ESCAPE_KIND_NO_LEAVE
                || g_btl_debug_flags[0] != 0) {
                got = 1;
                g_btl_delay = wait;
                g_btl_step = 2;
                break;
            }
            if (g_btl_no_escape != 0
                || g_btl_enemy_level - g_btl_party_level >= 10
                || g_btl_party_no_flee != 0) {
                got = 0;
                g_btl_delay = wait;
                g_btl_step = 2;
                break;
            }
            for (i = 0; i < BTL_PARTY; i++) {
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & ESCAPE_SPARED) == 0
                    && BtlStatusStops(&g_btl_actors[i]) != 0) {
                    break;
                }
            }
            if (i >= BTL_PARTY) {
                got = 0;
                g_btl_delay = wait;
                g_btl_step = 2;
                break;
            }
            for (i = 0; i < ESCAPE_ENEMIES; i++) {
                if (g_btl_combatants[i].c.key != 0
                    && BtlStatusStops(&g_btl_combatants[i]) != 0) {
                    break;
                }
            }
            if (i >= ESCAPE_ENEMIES) {
                got = 1;
                g_btl_delay = wait;
                g_btl_step = 2;
                break;
            }
            chance = 0;
            n = (g_btl_party_agility + g_btl_party_luck) / 2
                   - (g_btl_enemy_agility + g_btl_enemy_luck) / 2;
            for (i = 0; i < ESCAPE_EDGES; i++) {
                if (n >= D_800CFCAC[i]) {
                    chance = D_800CFCB4[i];
                    break;
                }
            }
            g_btl_delay = wait;
            presses = 0;
            g_btl_step++;
            break;
        case 1:
            if (g_btl_delay != 0) {
                if ((g_btl_pad1_edge & g_btl_key_confirm) != 0) {
                    presses++;
                    g_btl_escape_presses = presses;
                }
                break;
            }
            for (i = 0, rank = ESCAPE_ROWS, n = bonus + chance; i < ESCAPE_ROWS; i++) {
                if (D_800CFCBC[i] >= n) {
                    rank = i;
                    break;
                }
            }
            for (i = 0, col = ESCAPE_COLS; i < ESCAPE_COLS; i++) {
                if (D_800CFCC8[i] >= presses) {
                    col = i;
                    break;
                }
            }
            n += D_800CFCD0[rank][col];
            n = n < 0 ? 0
                 : n > ESCAPE_CHANCE_MAX ? ESCAPE_CHANCE_MAX : n;
            if ((int)(rand() & ESCAPE_ROLL_MASK) < n) {
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
            n = 1;
            do {
                n++;
                BtlDrawFrame();
            } while (n != 7);
            BtlOpenMessage(0, 0, line, ESCAPE_MSG_WIDTH, ESCAPE_MSG_STYLE);
            if (got == 0) {
                g_btl_delay = ESCAPE_HELD;
                g_btl_step++;
            } else {
                g_btl_step += 2;
            }
            break;
        case 3:
            if (g_btl_delay != 0) {
                break;
            }
            for (i = 0; i < BTL_PARTY; i++) {
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & ESCAPE_SPARED) == 0) {
                    g_btl_actors[i].flags |= ESCAPE_CAUGHT_BIT;
                }
            }
            BtlPickSettle();
            BtlPartyResetGfx();
            BtlCloseMessage(0);
            g_btl_battle_kind = ESCAPE_KIND_CAUGHT;
            return 1;
        case 4:
            BtlHideMarkers();
            for (i = 0, n = 0; i < BTL_PARTY; i++) {
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & ESCAPE_SPARED) == 0) {
                    row = &g_btl_member_scripts[SCRIPT_LEAVE
                        + g_btl_actors[i].c.key * MEMBER_SCRIPT_MODEL];
                    BtlObjSetScript(
                        g_btl_actors[i].obj,
                        (BtlSeqStep *)g_btl_actors[i].obj->scripts[
                            row[g_btl_actors[i].script_pick
                                * MEMBER_SCRIPT_PICK]]);
                    g_btl_actors[i].obj->motion = ESCAPE_MOTION;
                    g_btl_actors[i].obj->timer = ESCAPE_DELAY + n * ESCAPE_STAGGER;
                    n++;
                    g_btl_actors[i].flags |= ESCAPE_LEAVE_BIT;
                }
            }
            g_btl_step++;
            break;
        case 5:
            for (i = 0, n = 1; i < BTL_PARTY; i++) {
                if (g_btl_actors[i].c.key != 0
                    && (g_btl_actors[i].flags & ESCAPE_LEAVE_BIT) != 0
                    && g_btl_actors[i].obj->motion != 0) {
                    n = 0;
                    break;
                }
            }
            if (n == 0) {
                break;
            }
            for (i = 0; i < BTL_PARTY; i++) {
                if (g_btl_actors[i].c.key != 0
                    && (g_btl_actors[i].flags & ESCAPE_LEAVE_BIT) != 0) {
                    g_btl_actors[i].obj->attr &= ~ESCAPE_HIDDEN;
                }
            }
            do {
                BtlDrawFrame();
            } while (BtlMarkersHidden() == 0);
            g_btl_stage = ESCAPE_STAGE_LEFT;
            return 1;
        }
        BtlDrawFrame();
    } while (1);
}
