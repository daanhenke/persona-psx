/* Persona 1 (JP) - writing one fighter's whole palette white.  BTLP only.
 *   0x8008B188 BtlWhitenActorClut
 *
 * The same thing BtlFxStep86 does inline for a warded fighter, written as a
 * routine: every entry of the slot's palette but the first goes to white and
 * the slot is put on the fading list, so BtlStepCluts walks it back down
 * afterwards. A record that is held or that follows the camera is left alone,
 * and so is one whose slot the list is already walking.
 *
 * Nothing in the overlay reaches it - it is in the bytes and no call and no
 * table points at it - which is why the boundary finder left it labelled as
 * data.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>

/* A palette is 256 entries; entry nought is not written. */
#define CLUT_ENTRIES 0x100
#define CLUT_WHITE   0xFFFF

extern u_short  g_btl_clut_fading;
extern u_short *g_btl_actor_clut;

void BtlWhitenActorClut(BtlObj *o)
{
    int slot;
    int i;

    slot = o->mark_num;
    if ((o->attr & (BTL_OBJ_HELD | BTL_OBJ_TRACKING)) != 0) {
        return;
    }
    if (((g_btl_clut_fading >> slot) & 1) != 0) {
        return;
    }
    g_btl_clut_fading |= 1 << slot;
    i = 1;
    do {
        g_btl_actor_clut[slot * CLUT_ENTRIES + i] = CLUT_WHITE;
        i++;
    } while (i < CLUT_ENTRIES);
}
