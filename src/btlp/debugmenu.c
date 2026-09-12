/* Persona 1 (JP) - the page the debug HUD opens instead of the orders menu.
 * BTLP only.
 *   0x8009F724 BtlDebugMenu  0x8009F97C BtlDebugLoadGfx
 *   0x8009F9A0 BtlDebugSummon
 *
 * BtlOrdersMenu hands straight to this when the cancel key is held with
 * g_btl_debug_hud raised, so on disc - where that flag is zero - none of it is
 * reachable. It is a fifteen-row grid of switches over g_btl_debug_actions,
 * with a cursor that moves through a table of neighbours rather than by
 * counting, and it answers zero so the round carries on as though nothing had
 * been ordered.
 *
 * Five of the fifteen rows have a routine behind them; the two named here are
 * the shortest of those. Two more are handled in place because they change a
 * number rather than run anything: one steps the scene's music index round its
 * 0x43 values and rewrites the two hex digits the board shows it with, and one
 * hands over an item. Row 6 is the one guarded by a modifier - it only takes
 * the key with one of the three shoulder-side buttons held.
 *
 * The row is chosen on one turn of the loop and acted on the next, which is
 * why it outlives the step it was picked in.
 *
 * BtlDebugLoadGfx has no reference anywhere: the table entry that reached it
 * is zero. It is the debug handlers' shape exactly - do the one thing, answer
 * zero - and it reads the leader's artwork back in, so it is named for that.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>

/* One routine per row, zero where the row is handled in place or does
   nothing. */
extern int (*g_btl_debug_actions[])(void);

/* The two digits the board shows the music index with. */
extern u_char g_btl_debug_hex[];
#define DEBUG_HEX_DIGITS 2

/* Runs the cursor over the grid. Answers the row on a confirm, -1 on a
   cancel and BTL_PICK_WAIT while nothing has been decided - the same contract
   BtlPickUpdate keeps. */
extern int BtlDebugUpdate(void);

/* The scene's music index, which this page is the only thing that edits.

   The two spellings are both in the bytes. The line put up before the loop
   reaches it through the extern, so its address arrives with a relocation;
   the act step reaches it by literal address, which is the only way the whole
   address lands in a saved register of its own - and it is read and written
   through that register every time the row comes round. */
extern u_char g_map_unk4;
#define g_map_bgm (*(u_char *)0x801F5354)
#define DEBUG_BGM_COUNT 0x43

/* The three rows that are not ordinary switches. */
#define DEBUG_ROW_GUARDED 6      /* wants a shoulder button held with it */
#define DEBUG_ROW_ITEM    0xB
#define DEBUG_ROW_BGM     0xE

/* What the item row hands over, and the mask the guarded row wants. */
#define DEBUG_ITEM      0x10
#define DEBUG_GUARD_KEYS 7

/* Steps of the page: taking a row, acting on it, and letting the markers
   come back before the round is given the frame again. */
#define DEBUG_STEP_PICK 0
#define DEBUG_STEP_ACT  1
#define DEBUG_STEP_DONE 2

int BtlDebugMenu(void)
{
    int row;
    int bgm;

    BtlDrawFrame();
    BtlFormatHex(g_btl_debug_hex, g_map_unk4, DEBUG_HEX_DIGITS);
    BtlPickSettle();
    BtlRetractMarkers();
    BtlOpenDebugBoard();

    for (;;) {
        switch (g_btl_step) {
        case DEBUG_STEP_PICK:
            row = BtlDebugUpdate();
            if (g_btl_leave_round != 0) {
                row = -1;
            }
            if (row != BTL_PICK_WAIT) {
                if (row == -1) {
                    BtlSePlay(1, 2);
                    BtlCloseDebugBoard();
                    BtlRefreshMarkers();
                    BtlPickRefresh();
                    BtlPickHighlight(g_btl_pick_help_row);
                    g_btl_step = DEBUG_STEP_DONE;
                } else if (row != DEBUG_ROW_GUARDED
                           || (g_btl_pad1 & DEBUG_GUARD_KEYS) != 0) {
                    g_btl_step++;
                }
            }
            break;

        case DEBUG_STEP_ACT:
            /* The row is looked up again for the call. Taking it into a
               local instead loses the second read the image has. */
            if (g_btl_debug_actions[row] != 0) {
                g_btl_step = DEBUG_STEP_PICK;
                BtlCloseDebugBoard();
                g_btl_debug_actions[row]();
                BtlOpenDebugBoard();
                g_btl_step = DEBUG_STEP_PICK;
            } else if (row == DEBUG_ROW_BGM) {
                bgm = g_map_bgm;
                bgm++;
                if (bgm >= DEBUG_BGM_COUNT) {
                    bgm = 0;
                }
                g_map_bgm = bgm;
                BtlFormatHex(g_btl_debug_hex, bgm, DEBUG_HEX_DIGITS);
                g_btl_step--;
            } else if (row == DEBUG_ROW_ITEM) {
                BtlSePlay(2, 9);
                BtlGiveItem(DEBUG_ITEM);
                g_btl_step--;
            } else {
                g_btl_step = DEBUG_STEP_PICK;
            }
            break;

        case DEBUG_STEP_DONE:
            if (BtlMarkersIdle() != 0) {
                return 0;
            }
            break;
        }
        BtlDrawFrame();
    }
}

int BtlDebugLoadGfx(void)
{
    BtlLoadActorGfx(0);
    return 0;
}

/* Row 4: puts the leader's object on the motion that reads the Persona in and
   plays the summoning through, which is the one thing on this page that has
   to be started rather than done. */
int BtlDebugSummon(void)
{
    g_btl_actors[0].obj->motion = 5;
    return 0;
}
