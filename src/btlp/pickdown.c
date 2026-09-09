/* Persona 1 (JP) - picking one of the fallen.  BTLP only.
 *   0x8009B4B8 BtlPickDownMember
 *
 * One frame of it, with the cursor's slot passed by pointer so the caller can
 * hold it between frames. The sideways keys walk the cursor over the members
 * who are down - which is the opposite of the picker next door, and the reason
 * this one exists at all.
 *
 * The answer is the slot on confirm, -1 on cancel and -2 on the third key,
 * with the party's graphics put back first; -0x100 means the player has not
 * decided yet. A cursor already off the end answers -1 to any key at all,
 * which is how a pick with nobody to pick is backed out of.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/sound.h>


int BtlPickDownMember(short *slot)
{
    int keys;

    keys = BtlMenuKey();
    if ((keys & PAD_LEFT) != 0) {
        *slot = BtlDownMemberPrev(*slot);
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
    }
    if ((keys & PAD_RIGHT) != 0) {
        *slot = BtlDownMemberNext(*slot);
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
    }
    if (*slot < 0) {
        if (g_btl_pad1_edge != 0) {
            return BTL_PICK_CANCEL;
        }
        return BTL_PICK_WAIT;
    }
    if ((g_btl_pad1_edge & g_btl_key_confirm) != 0) {
        BtlPartyResetGfx();
        return *slot;
    }
    if ((g_btl_pad1_edge & g_btl_key_cancel) != 0) {
        BtlPartyResetGfx();
        return BTL_PICK_CANCEL;
    }
    if ((g_btl_pad1_edge & g_btl_key_abort) != 0) {
        BtlPartyResetGfx();
        return BTL_PICK_ABORT;
    }
    return BTL_PICK_WAIT;
}
