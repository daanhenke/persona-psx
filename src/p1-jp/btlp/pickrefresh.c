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
#include <types.h>
#include <persona/btlp/object.h>

/* Commands to a page. */
#define PICK_SLOTS 6

/* How bright a command is drawn, usable or not. */
#define PICK_DARK 0x20
#define PICK_LIVE 0x80

/* Arrive in one frame, and the motion they all sit in. */
#define PICK_FADE   0xFF
#define PICK_MOTION 3

/* Where the help line is put. */
#define PICK_HELP_X 0x10
#define PICK_HELP_Y 0x94

extern BtlObj      *g_btl_pick_objs[];
extern const u_char g_btl_pick_live[][PICK_SLOTS];
extern u_char       g_btl_pick_page;
extern const char  *g_btl_pick_help[];
extern short        g_btl_pick_help_row;
extern short        g_btl_pick_help_row2;
extern u_char       g_btl_no_help;

extern void BtlObjSetRgb(BtlObj *obj, short r, int g, short b);
extern void BtlObjSetFade(BtlObj *obj, u_char rate);
extern void BtlObjSetTimer(BtlObj *obj, short timer);
extern void BtlObjSetMotion(BtlObj *obj, u_char motion);
extern void BtlOpenMessage(int a, int b, const char *text, int x, int y);

void BtlPickRefresh(void)
{
    const u_char (*live)[PICK_SLOTS];
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
    } while (i < PICK_SLOTS);

    if (g_btl_no_help == 0) {
        help = g_btl_pick_help + g_btl_pick_page * PICK_SLOTS;
        if (g_btl_pick_page != 0) {
            text = help[g_btl_pick_help_row2];
        } else {
            text = help[g_btl_pick_help_row];
        }
        BtlOpenMessage(0, 0, text, PICK_HELP_X, PICK_HELP_Y);
    }
}
