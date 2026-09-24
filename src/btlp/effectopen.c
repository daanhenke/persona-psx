/* Persona 1 (JP) - taking a slot for an effect.  BTLP only.
 *   0x80077534 BtlEffectOpen
 *
 * Four slots hold the effects the draw pass walks. This finds a free one for a
 * record, puts the record back to its opening state and works out which step
 * of it the draw pass should start on.
 *
 * The steps are chained through the record's `next`, ending at -1. Marked
 * records are ones another effect has already taken over, so the chain is
 * walked to the first unmarked one and that is what g_btl_effect_step keeps;
 * an empty chain, or one whose records are all marked, leaves it at -1.
 *
 * A record that already holds a slot is refused with 0x100 rather than opened
 * twice, and with all four slots taken the answer is -1.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/effect.h>
#include <persona/btlp/input.h>

/* Where the effect artwork sits in VRAM. */
#define BTL_EFFECT_TPX 0x3C0
#define BTL_EFFECT_TPY 0x100

/* What a fresh record is put back to. */
#define BTL_EFFECT_UNITY 0x1000

/* Handed back when the record already holds a slot. */
#define BTL_EFFECT_TAKEN 0x100

/* The kept copy of the draw mode is one structure assignment. gcc emits that
   as a single block-move insn, which reorg will not put in a delay slot, so
   the jump out of the slot loop fills its slot from the target instead. */
int BtlEffectOpen(BtlEffect *e)
{
    BtlEffectRow *node;
    int           i;
    int           took;

    for (i = 0; i < BTL_EFFECT_SLOTS; i++) {
        if (g_btl_effect[i] == e) {
            return BTL_EFFECT_TAKEN;
        }
    }

    if (i == BTL_EFFECT_SLOTS) {
        for (i = 0; i < BTL_EFFECT_SLOTS; i++) {
            if (g_btl_effect[i] == (BtlEffect *)BTL_EFFECT_FREE) {
                BtlCursorInitPrims();
                node = (BtlEffectRow *)e;
                g_btl_effect[i] = e;
                g_btl_effect_step[i] = (BtlEffectRow *)BTL_EFFECT_FREE;
                while (node->next != (BtlEffectRow *)BTL_EFFECT_FREE) {
                    node = node->next;
                    if (node->row == 0) {
                        g_btl_effect_step[i] = node;
                        break;
                    }
                }
                e->answer = BTL_EFFECT_MARK;
                e->unk28 = 100;
                e->scale_x = 0x10;
                e->scale_y = 0x40;
                e->kind = 0;
                e->sel = 0;
                e->unk20 = 0;
                e->unk24 = 0;
                e->unk30 = 0;
                e->unk32 = 0;
                e->unk34 = 0;
                e->scale = BTL_EFFECT_UNITY;
                e->flags |= BTL_EFFECT_RUNNING;
                SetDrawMode((DR_MODE *)e->mode, 0, 0,
                            GetTPage(0, 0, BTL_EFFECT_TPX, BTL_EFFECT_TPY), 0);
                *(DR_MODE *)e->mode_kept = *(DR_MODE *)e->mode;
                break;
            }
        }
    }

    took = -1;
    if (i != BTL_EFFECT_SLOTS) {
        took = i;
    }
    return took;
}

