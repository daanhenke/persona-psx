/* Persona 1 (JP) - the front slots' open and close.  BTLP only.
 *   0x80065FC4 BtlFrontSlotGrow
 *   0x80066084 BtlFrontSlotShrink
 *   0x80066148 BtlFrontSlotStep
 *   0x80066150 BtlFrontSlotDraw
 *   0x80066158 BtlFrontSlotsTick
 *   0x80066254 BtlFrontQuadUv
 *
 * Each of the four slots has a mode word: its low byte says whether the slot
 * is opening (1) or closing (2), and the 0x200 bit says the scale has still
 * to be started. Opening starts both scales at 0x1000 and grows the second by
 * half again each frame until it reaches 0x1000; closing starts at 2 and 0x40
 * and halves the second until it is back to 0x40, then zeroes both. Once a
 * slot has finished closing its mode is cleared.
 *
 * Nothing in the overlay calls the tick or the UV helper; the step and the
 * draw it would hand a shown slot to are both empty.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/front.h>

/* Both answer whether the slot is still moving. The answer is set inside each
   arm, ahead of the stores, so reorg can hand it to the branch's delay slot. */
int BtlFrontSlotGrow(int slot)
{
    int going;

    if (g_btl_front_modes[slot] & BTL_FRONT_MODE_START) {
        BTL_FRONT_SLOTS[slot].unk54 = 0x1000;
        BTL_FRONT_SLOTS[slot].unk58 = 0x1000;
    }
    BTL_FRONT_SLOTS[slot].unk58 += BTL_FRONT_SLOTS[slot].unk58 / 2;
    if (BTL_FRONT_SLOTS[slot].unk58 >= 0x1000) {
        going = 0;
        BTL_FRONT_SLOTS[slot].unk54 = 0x1000;
        BTL_FRONT_SLOTS[slot].unk58 = 0x1000;
        return going;
    }
    return 1;
}

int BtlFrontSlotShrink(int slot)
{
    int going;

    if (g_btl_front_modes[slot] & BTL_FRONT_MODE_START) {
        BTL_FRONT_SLOTS[slot].unk54 = 2;
        BTL_FRONT_SLOTS[slot].unk58 = 0x40;
    }
    BTL_FRONT_SLOTS[slot].unk58 -= BTL_FRONT_SLOTS[slot].unk58 / 2;
    if (BTL_FRONT_SLOTS[slot].unk58 > 0x40) {
        going = 1;
    } else {
        going = 0;
        BTL_FRONT_SLOTS[slot].unk54 = 0;
        BTL_FRONT_SLOTS[slot].unk58 = 0;
    }
    return going;
}

void BtlFrontSlotStep(int slot)
{
}

void BtlFrontSlotDraw(int slot, u_long *ot)
{
}

void BtlFrontSlotsTick(u_long *ot)
{
    int i;
    u_char mode;

    for (i = 0; i < BTL_FRONT_SLOT_COUNT; i++) {
        mode = g_btl_front_modes[i];
        /* Nought is tested on its own: with it inside, the switch has three
           values and gcc splits them into a tree rather than a chain. */
        if (mode == 0) {
            continue;
        }
        switch (mode) {
        case 1:
            BtlFrontSlotGrow(i);
            break;
        case 2:
            if (BtlFrontSlotShrink(i) == 0) {
                g_btl_front_modes[i] = 0;
            }
            break;
        default:
            g_btl_front_modes[i] = 0;
            break;
        }
        if (BTL_FRONT_SLOTS[i].flags & 0x8000) {
            BtlFrontSlotStep(i);
            BtlFrontSlotDraw(i, ot);
        }
    }
    g_btl_front_slot_phase ^= 1;
}

/* Eight bytes of nothing between the tick and the UV helper, called from
   nowhere. */
void func_8006624C(void)
{
}

/* Sets a quad's texture corners to a u, v, w, h box, a texel in from the edge
   a flipped quad would otherwise sample past, and never past 0xFF. */
void BtlFrontQuadUv(POLY_FT4 *p, int u, int v, int w, int h)
{
    int dx;
    int dy;

    dx = p->x1 - p->x0;
    dy = p->y1 - p->y0;
    if (dx < 0) {
        u--;
        if (u < 0) {
            u = 0;
            w--;
        }
    }
    if (dy < 0) {
        v--;
        if (v < 0) {
            v = 0;
            h--;
        }
    }
    if (u + w >= 0x100) {
        w--;
    }
    if (v + h >= 0x100) {
        h--;
    }
    p->u0 = u;
    p->v0 = v;
    p->u1 = u + w;
    p->v1 = v;
    p->u2 = u;
    p->v2 = v + h;
    p->u3 = u + w;
    p->v3 = v + h;
}
