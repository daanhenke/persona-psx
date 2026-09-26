/* Persona 1 (JP) - an effect object on every fighter the move reaches.
 * BTLP only.
 *   0x800C021C BtlOpenFxOnTargets
 *
 * Walks both sides against the acting fighter's target mask and opens one
 * effect record over each fighter it finds, chaining them together so the
 * whole set can be handled as one. Each record is given a later timer than
 * the one before it, which is what staggers the effect across the field.
 *
 * The fighter under each record is put on the colour it is handed before the
 * record is made, so the effect and the tint arrive together.
 *
 * The head of the chain is the one answered. A move whose mask reaches nobody
 * never sets it, and the caller is handed whatever the register held - the
 * mask is filled in from a live target, so it does not happen.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How fast the fighter under a record walks toward the effect's colour. */
#define FX_FADE 2

/* How much later each record starts than the one before it. */
#define FX_STEP 2

/* 93.64%. The image stores r, g and b as words into three slots 8 bytes
   apart (0x10/0x18/0x20, a 0x50 frame) and reads each back with lhu. Traced
   through the gcc 2.6.0 source (build/gcc-src):
   - only alignment -1 spaces slots by 8: reload's spill slots (alter_reg) or
     BLKmode objects (assign_stack_local);
   - one pseudo can never give sw + lhu: with LOAD_EXTEND_OP defined, reload
     reloads any size-changing subreg of a pseudo in the pseudo's own mode
     (reload.c push_reload and find_reloads' force_reload), so an int
     parameter reloads with lw and a narrow one is stored with sh;
   - a BLKmode short[2] per colour, written as a word, reproduces the frame,
     the slots, the stores and the loads. That is the form below.
   Left: the image loads into t0 with the object pointer in v0, where local
   alloc here gives the value v0 and the pointer v1. t0 is the register reload
   picks, which means the image's load pseudo got no hard reg and was replaced
   by its memory equivalent (update_equiv_regs route A) - what keeps local
   alloc off it is still open. One variable reused for all three puts the
   pointer in v0 but the value in v1 (88.74%). Also open: the timer copy
   (addu s4,a3) sits after s1's setup in the image. */
#ifdef NON_MATCHING
BtlObj *BtlOpenFxOnTargets(int r, int g, int b, int timer)
{
    BtlObj  *o;
    BtlObj  *head;
    BtlObj  *last;
    int      slot;
    int      bit;
    u_short  targets;
    short    cr[2], cg[2], cb[2];

    *(int *)cr = r;
    *(int *)cg = g;
    *(int *)cb = b;
    bit     = 1;
    slot    = 0;
    last    = 0;
    targets = g_btl_actors[g_btl_actor_turn].targets;

    for (; slot < BTL_ACTORS; slot++, bit <<= 1)
    {
        if (g_btl_actors[slot].c.key != 0
            && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
            && !(g_btl_actors[slot].flags & BTL_ACTOR_OUT)
            && (targets & bit) != 0)
        {
            g_btl_actors[slot].obj->rgb_to[0] = cr[0];
            g_btl_actors[slot].obj->rgb_to[1] = cg[0];
            g_btl_actors[slot].obj->rgb_to[2] = cb[0];
            g_btl_actors[slot].obj->fade      = FX_FADE;

            o = BtlOpenFxObj(slot, timer);
            if (last != 0)
            {
                last->attached = o;
                o->mark_num    = FX_MARK_REST;
            }
            else
            {
                o->mark_num = FX_MARK_HEAD;
                head        = o;
            }
            timer += FX_STEP;
            last = BtlObjLast(o);
        }
    }
    return head;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxtargets", BtlOpenFxOnTargets);
#endif
