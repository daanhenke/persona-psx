/* Persona 1 (JP) - showing which of the six slots is chosen.  BTLP only.
 *   0x800A9D24 BtlPickHighlight
 *
 * The picker is six objects in a row, each carrying a second one on its
 * attached link. The object is the item and the attached piece is the
 * highlight behind it, which is why the two are coloured separately: setting
 * the item's colour walks the chain and reaches the highlight as well, so the
 * highlight is written again afterwards.
 *
 * A slot that cannot be chosen is drawn dark and its highlight is left where
 * it was; the rest are drawn at full and their highlights sit at 0x60, with
 * the chosen one at 0xFF.
 */
#include <types.h>
#include <persona/btlp/object.h>

/* How bright a slot's item is drawn, live or not. */
#define PICK_DARK 0x20
#define PICK_LIVE 0x80

/* And its highlight, chosen or not. */
#define PICK_CHOSEN 0xFF
#define PICK_REST   0x60

/* Arrive in one frame. */
#define PICK_FADE 0xFF

extern BtlObj      *g_btl_pick_objs[];
extern const u_char g_btl_pick_live[][6];
extern u_char       g_btl_pick_page;

extern void BtlObjSetRgb(BtlObj *obj, short r, int g, short b);
extern void BtlObjSetFade(BtlObj *obj, u_char rate);

void BtlPickHighlight(int chosen)
{
    int i;

    i = 0;
    do {
        if (g_btl_pick_live[g_btl_pick_page][i] == 0) {
            BtlObjSetRgb(g_btl_pick_objs[i], PICK_DARK, PICK_DARK, PICK_DARK);
        } else {
            BtlObjSetRgb(g_btl_pick_objs[i], PICK_LIVE, PICK_LIVE, PICK_LIVE);
        }
        BtlObjSetFade(g_btl_pick_objs[i], PICK_FADE);

        if (chosen == i) {
            g_btl_pick_objs[i]->attached->rgb_to[0] = PICK_CHOSEN;
            g_btl_pick_objs[i]->attached->rgb_to[1] = PICK_CHOSEN;
            g_btl_pick_objs[i]->attached->rgb_to[2] = PICK_CHOSEN;
            g_btl_pick_objs[i]->attached->fade = PICK_FADE;
        } else if (g_btl_pick_live[g_btl_pick_page][i] != 0) {
            g_btl_pick_objs[i]->attached->rgb_to[0] = PICK_REST;
            g_btl_pick_objs[i]->attached->rgb_to[1] = PICK_REST;
            g_btl_pick_objs[i]->attached->rgb_to[2] = PICK_REST;
            g_btl_pick_objs[i]->attached->fade = PICK_FADE;
        }
        i++;
    } while (i < 6);
}
