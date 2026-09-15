/* Persona 1 (JP) - posting to the layer in front of the battle.  BTLP only.
 *   0x80065D30 BtlFrontSlotSet
 *   0x80065D64 BtlFrontPost
 *
 * BtlFrontSlotSet writes the two words at 0x10 in one of the front slots; the
 * offer scene clears slot nought's with it before it plays.
 *
 * BtlFrontPost is the only thing in the overlay that ever starts the front
 * step BtlDrawFront would run, and nothing in the overlay calls it. With slot
 * nought's 0x400 bit clear it appends the id to the fifteen-entry queue - the
 * first post also uploads the palette block and sets the step going - and
 * answers nought; with the bit set it hands the id to the slot itself and
 * answers one.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/front.h>

void BtlFrontSlotSet(int slot, int a, int b)
{
    BTL_FRONT_SLOTS[slot].unk10 = a;
    BTL_FRONT_SLOTS[slot].unk14 = b;
}

int BtlFrontPost(int id)
{
    u_short flags;
    int i;

    flags = BTL_FRONT_SLOTS[0].flags;
    if (!(flags & 0x400)) {
        BTL_FRONT_SLOTS[0].flags = 0;
        for (i = 0; i < BTL_FRONT_QUEUE; i++) {
            if (g_btl_front_queue[i] == -1) {
                if (i == 0) {
                    BtlQueueVramLoad(g_btl_front_clut, 0x300, 0x1E8, 0x10, 8);
                    g_btl_front_step = 1;
                    g_btl_front_timer = 15;
                }
                g_btl_front_queue[i] = id;
                g_btl_front_queue[i + 1] = -1;
                break;
            }
        }
        g_btl_front_flag = 0;
        return 0;
    }
    BTL_FRONT_SLOTS[0].unk20 = 0;
    BTL_FRONT_SLOTS[0].flags = (flags & ~0x410) | 0xE4;
    BTL_FRONT_SLOTS[0].unk28 = id;
    return 1;
}

/* Eight bytes of nothing between BtlFrontPost and BtlDrawFront, called from
   nowhere. */
void func_80065E6C(void)
{
}
