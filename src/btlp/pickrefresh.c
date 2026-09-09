/* Persona 1 (JP) - redrawing the battle command menu.  BTLP only.
 *   0x800A9A08 BtlPickRefresh
 *
 * Six commands to a page, and a command the party cannot use this turn is
 * dimmed rather than taken away, so the menu keeps its shape: the six objects
 * are given full white or a quarter grey depending on g_btl_pick_live, then
 * all put on the same motion with the fade wide open so the change arrives in
 * one frame.
 *
 * The line under the menu is the command's own text out of g_btl_pick_help,
 * six entries a page, and the two pages keep their cursor in separate
 * variables rather than in a pair.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>

/* Arrive in one frame, and the motion they all sit in. */
#define PICK_MOTION 3

/* The other motion the six are put through, and where it settles them. */
#define PICK_SETTLE_MOTION 4
#define PICK_SETTLE_PHASE  1
#define PICK_SCALE_NORMAL  0x100

extern void BtlOpenMessage(int a, int b, const char *text, int x, int y);

#ifdef NON_MATCHING
void BtlPickRefresh(void)
{
    const u_char (*live)[BTL_PICK_SLOTS];
    const char **help;
    BtlObj **slot;
    int      i;
    const char *text;

    i = 0;
    live = g_btl_pick_live;
    slot = g_btl_pick_objs;
    do {
        if (live[g_btl_pick_page][i] == 0) {
            BtlObjSetRgb(*slot, PICK_DARK, PICK_DARK, PICK_DARK);
        } else {
            BtlObjSetRgb(*slot, PICK_LIVE, PICK_LIVE, PICK_LIVE);
        }
        i++;
        BtlObjSetFade(*slot, PICK_FADE);
        BtlObjSetTimer(*slot, 0);
        BtlObjSetMotion(*slot, PICK_MOTION);
        slot++;
    } while (i < BTL_PICK_SLOTS);

    if (g_btl_no_help == 0) {
        help = g_btl_pick_help + g_btl_pick_page * BTL_PICK_SLOTS;
        if (g_btl_pick_page != 0) {
            text = help[g_btl_pick_help_row2];
        } else {
            text = help[g_btl_pick_help_row];
        }
        BtlOpenMessage(0, 0, text, PICK_HELP_X, PICK_HELP_Y);
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/pickrefresh", BtlPickRefresh);
#endif

/* Puts all six back to their resting state: full size and, once the motion
   runs out, full colour again. Phase 1 starts it on the second half of motion
   4, which is the half that shrinks the height and then restores the colour -
   the first half, which works on the width, is skipped. */
void BtlPickSettle(void)
{
    BtlObj **slot;
    int      i;

    i = 0;
    slot = g_btl_pick_objs;
    do {
        BtlObjSetTimer(*slot, 0);
        i++;
        BtlObjSetPhase(*slot, PICK_SETTLE_PHASE);
        BtlObjSetScaleTo(*slot, PICK_SCALE_NORMAL);
        BtlObjSetMotion(*slot, PICK_SETTLE_MOTION);
        slot++;
    } while (i < BTL_PICK_SLOTS);
}
