/* Persona 1 (JP) - showing which of the six slots is chosen.  BTLP only.
 *   0x800A9D24 BtlPickHighlight
 *
 * Each slot is two objects. BtlPickSpawn builds them the other way round from
 * how they read here: the one in g_btl_pick_objs comes from a single template
 * shared by all six, and what hangs off its attached link is the per-slot
 * piece. So the shared one is the frame and the attached one is the item.
 *
 * They have to be coloured separately because setting the frame's colour walks
 * the chain and reaches the item too, which is why the item is written again
 * afterwards.
 *
 * A slot that cannot be chosen is drawn dark and its item is left where it
 * was; the rest are drawn at full with their items at 0x60, and the chosen
 * one's item at 0xFF.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>

/* How bright a slot's item is drawn, chosen or not. */
#define PICK_CHOSEN 0xFF
#define PICK_REST   0x60

/* The colour is set up as in BtlPickRefresh: the levels and the object into
   locals, in the order the image loads them. */
void BtlPickHighlight(int chosen)
{
    int i;
    short red, blue;
    int green;
    BtlObj *obj;

    i = 0;
    do {
        if (g_btl_pick_live[g_btl_pick_page][i] == 0) {
            red = PICK_DARK;
            green = PICK_DARK;
            obj = g_btl_pick_objs[i];
            blue = PICK_DARK;
        } else {
            red = PICK_LIVE;
            green = PICK_LIVE;
            obj = g_btl_pick_objs[i];
            blue = PICK_LIVE;
        }
        BtlObjSetRgb(obj, red, green, blue);
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
