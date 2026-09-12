/* Persona 1 (JP) - looking an enemy over.  BTLP only.
 *   0x8009F638 BtlAnalyseMenu
 *
 * The last of the six in g_btl_pick_command, and the only one of them that
 * neither gives an order nor opens a menu: it puts the status board up, runs
 * the enemy cursor over the field, and rewrites the board from whichever
 * enemy the cursor is on. Nothing is chosen at the end of it - confirming
 * only closes the board again - so it answers zero the way the others do when
 * the party is left where it was.
 *
 * Three frames go by before the cursor is armed, which is how long the board
 * takes to grow into place, and thirty after the board is shut, which is how
 * long it takes to go away again. The analysis is rewritten every turn of the
 * loop rather than only when the cursor moves, so it is drawn from the slot
 * whether or not the pick settled on one.
 *
 * The whole routine is one loop with the frame at the bottom, which is what
 * puts BtlDrawFrame past everything else rather than under the prologue.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/text.h>

/* Frames the board is given to go away in. */
#define ANALYSE_CLOSE 30

int BtlAnalyseMenu(void)
{
    int wait;

    BtlCloseMessage(0);
    BtlRetractMarkers();
    BtlOpenStatusBoard();
    BtlDrawFrame();
    BtlDrawFrame();
    BtlDrawFrame();
    BtlSetPickable();
    g_btl_enemy_slot = BtlPickableNext(-1);

    for (;;) {
        if (g_btl_step == 0) {
            /* The one local carries the pick in and then counts the frames
               the board is shut over; two locals do not compile to this. */
            wait = BtlPickEnemy(&g_btl_enemy_slot);
            BtlShowEnemyStatus(g_btl_enemy_slot);
            if (wait != BTL_PICK_WAIT) {
                BtlSePlay(1, 2);
                BtlCloseStatusBoard();
                wait = 0;
                BtlRefreshMarkers();
                do {
                    wait++;
                    BtlDrawFrame();
                } while (wait < ANALYSE_CLOSE);
                return 0;
            }
        }
        BtlDrawFrame();
    }
}
