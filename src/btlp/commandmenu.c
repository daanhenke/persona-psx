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
#include <persona/btlp/round.h>
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

INCLUDE_ASM("btlp/nonmatchings/commandmenu", BtlOrdersMenu);
