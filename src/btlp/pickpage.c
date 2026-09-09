/* Persona 1 (JP) - putting a page of commands up.  BTLP only.
 *   0x800A9BC8 BtlPickShowPage
 *
 * The picker's six slots are re-used for every page: rather than build new
 * objects, each slot is handed the page's artwork, told whether it is one the
 * player may choose, and set going on the same motion. The help line under
 * the menu follows.
 *
 * It refuses while the slots are still moving - the frame object's motion is
 * the busy flag for all six - and says so with a zero, so a caller that turns
 * a page can tell whether the turn took.
 *
 * The page is recorded before that test rather than after, so a refused call
 * still leaves the new page number behind.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>

/* The motion every slot is put through as a page comes up. */
#define PICK_PAGE_MOTION 5

/* Two pointers a page, of which only the first is read here. */
extern const u_long *g_btl_pick_page_gfx[][2];

extern void BtlOpenMessage(int flags, short style, const u_char *script,
                           short x, short y);

int BtlPickShowPage(int page)
{
    const char **help;
    const char **text;
    int          i;

    g_btl_pick_page = page;
    if (g_btl_pick_objs[0]->motion != 0) {
        return 0;
    }

    for (i = 0; i < BTL_PICK_SLOTS; i++) {
        g_btl_pick_objs[i]->attached->unk60 =
            g_btl_pick_page_gfx[page * BTL_PICK_SLOTS + i][0];
        g_btl_pick_objs[i]->children = g_btl_pick_live[g_btl_pick_page][i];
        BtlObjSetMotion(g_btl_pick_objs[i], PICK_PAGE_MOTION);
    }

    if (g_btl_no_help != 0) {
        return 1;
    }

    help = g_btl_pick_help + g_btl_pick_page * BTL_PICK_SLOTS;
    if (g_btl_pick_page != 0) {
        text = help + g_btl_pick_help_row2;
    } else {
        text = help + g_btl_pick_help_row;
    }
    BtlOpenMessage(0, 0, (const u_char *)*text, PICK_HELP_X, PICK_HELP_Y);
    return 1;
}
