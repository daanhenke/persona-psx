/* Persona 1 (JP) - the menus a round opens with.  BTLP only.
 *   0x8009CA38 BtlConfigMenu    0x8009CCE4 BtlCommandEntry
 *   0x8009DD74 BtlRunTalkScene  0x8009DDC8 BtlOrdersMenu
 *
 * BtlStageCommand opens the round by asking the player what the party does,
 * and these are what it opens. Two of them are whole menus:
 *
 * BtlCommandEntry is the one that walks the party. It puts the pick cursor on
 * a member, takes a command for them, and moves on to the next, and it is the
 * only one of the five big enough to hold the whole of that.
 *
 * BtlOrdersMenu is the shorter way round: rather than a command each, the
 * party takes one standing order between them, and the three it can choose
 * are the same three a finished negotiation picks from - it hands straight to
 * BtlTalkersLeaveField or BtlTalkersJoin.
 *
 * BtlConfigMenu is the settings page. It walks a table of pointers, one per
 * row, to the settings themselves - g_btl_confirm is the first of them - and
 * left and right step each one through its own list of values rather than
 * counting, so a row can hold whatever values it likes.
 *
 * BtlRunTalkScene is the one-liner the other two lean on: it starts the
 * negotiation, takes the answer the scene ends with, and then turns the frame
 * over until every fighter has finished moving, so whoever called it comes
 * back to a still field.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/debug.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/status.h>
#include <persona/btlp/talk.h>

INCLUDE_ASM("btlp/nonmatchings/commandmenu", BtlConfigMenu);

INCLUDE_ASM("btlp/nonmatchings/commandmenu", BtlCommandEntry);

/* The answer is taken before the wait, not after: the scene is finished with
   by then and the field is only being let catch up. */
int BtlRunTalkScene(void)
{
    int outcome;

    BtlTalkStart();
    outcome = BtlTalkSceneStep();
    BtlPackEnemyGrid();
    while (BtlActorsIdle() == 0) {
        BtlDrawFrame();
    }
    return outcome;
}

/* The steps of the orders menu: taking an order, carrying it out once the
   markers have settled, and the tactics board the order can lead to. */
#define ORDERS_PICK    0
#define ORDERS_APPLY   1
#define ORDERS_TACTICS 2
#define ORDERS_JOIN    3

/* The three standing orders, in the order the board lists them. */
#define ORDER_STAY  0
#define ORDER_LEAVE 1
#define ORDER_JOIN  2

int BtlOrdersMenu(void)
{
    int order;

    if ((g_btl_pad1 & g_btl_key_cancel) && g_btl_debug_hud != 0) {
        BtlDebugMenu();
        return 0;
    }
    BtlRetractMarkers();
    BtlOpenOrdersBoard();
    for (;;) {
        BtlDrawFrame();
        switch (g_btl_step) {
        case ORDERS_PICK:
            order = BtlOrdersMenuUpdate();
            if (order == BTL_PICK_WAIT) {
                break;
            }
            if (order == -1) {
                if (BtlMarkersIdle() != 0) {
                    BtlSePlay(1, 2);
                    BtlCloseOrdersBoard();
                    BtlRefreshMarkers();
                    return 0;
                }
                break;
            }
            if (BtlMarkersIdle() == 0) {
                break;
            }
            BtlSePlay(1, 1);
            BtlCloseOrdersBoard();
            BtlRefreshMarkers();
            if (order != ORDER_JOIN) {
                BtlPickSettle();
            }
            g_btl_step++;
            break;

        case ORDERS_APPLY:
            if (BtlMarkersIdle() == 0) {
                break;
            }
            switch (order) {
            case ORDER_STAY:
                BtlTalkersStay();
                break;
            case ORDER_LEAVE:
                BtlTalkersLeaveField();
                break;
            case ORDER_JOIN:
                BtlTalkersJoin();
                break;
            }
            BtlShowAilmentMarks(0);
            return 1;

        case ORDERS_TACTICS:
            order = BtlTacticsMenuUpdate();
            if (order == BTL_PICK_WAIT) {
                break;
            }
            BtlCloseTacticsBoard();
            BtlRefreshMarkers();
            g_btl_step++;
            break;

        case ORDERS_JOIN:
            if (BtlMarkersIdle() == 0) {
                break;
            }
            BtlTalkersJoin();
            return 1;
        }
    }
}
