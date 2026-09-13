/* Persona 1 (JP) - the equipment names the fighter editor's board shows.
 * BTLP only.
 *   0x800A94C8 BtlEditEquipNames
 *
 * The debug page's fighter editor draws what a fighter is wearing as seven
 * lines of ten glyphs, and its board reads those lines out of
 * g_btl_edit_equip_names rather than out of the item table. This fills them:
 * a slot with something in it gets that item's name copied across, and an
 * empty one gets a line that ends before it starts.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/board.h>
#include <persona/common/item.h>

#define EQUIP_SLOTS 7
#define EQUIP_NAME  10

/* The seven lines the edit board draws the equipment with. */
extern u_char g_btl_edit_equip_names[EQUIP_SLOTS][EQUIP_NAME];

void BtlEditEquipNames(BtlActor *a)
{
    u_short *equip;
    int      i;

    equip = a->c.equip;
    for (i = 0; i < EQUIP_SLOTS; i++) {
        if (equip[i] != 0) {
            memcpy(g_btl_edit_equip_names[i], g_item_defs[equip[i]].name,
                   EQUIP_NAME);
        } else {
            g_btl_edit_equip_names[i][0] = BTL_TEXT_END;
        }
    }
}
