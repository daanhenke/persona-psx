/* Persona 1 (JP) - the move that opens one record over the far side of the
 * field.  BTLP only.
 *   0x800BA6B8 BtlFxStart38
 *
 * A start handler out of g_btl_spell_fx. One record on the centre line, eighty
 * units past the middle toward whichever side is being aimed at and sixteen
 * above the floor. It is neither hidden nor static, so it is drawn and its
 * script runs from the frame it is taken, and it starts black and is told to
 * walk to half grey - BtlFxStep36 then walks it back down again once its
 * timer runs out.
 *
 * The side is written straight into the position rather than through a local.
 * Taken into a local first, the two reaches for g_btl_fx_def fall into one
 * extended block and gcc shares the address between them; the image
 * materialises the template's address twice, which is what says they are not
 * in the same block.
 */
#include <decomp/include_asm.h>
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* Where the record stands: nothing across, eighty units past the middle on the
   side away from the one acting, and sixteen off the floor. */
#define FX_38_Y 0x500000
#define FX_38_Z 0x100000

/* What it starts as. Not the plain effect attribute: static, but neither
   hidden nor shadowed, and carrying the two bits the whitening also sets. */
#define FX_38_ATTR (BTL_OBJ_STATIC | BTL_OBJ_ATTR_4000 | 0x4 \
                    | BTL_OBJ_NO_SHADOW)

/* How long its glide lasts, how fast it takes on colour, and how long it
   stands before the step handler puts it out. */
#define FX_38_STEPS 0x20
#define FX_38_FADE  2
#define FX_38_TIMER 0x40

/* Fifty-three of the fifty-six instructions, and the whole tail from the call
   on. What is out is three slots of argument set-up: the image builds the
   position before it takes the template's address and gcc here takes the
   address first. Fourteen orderings of the three position stores, the
   template store and the side test move it no further, so this is the
   permuter's kind of residual rather than a shape one. */
#ifdef NON_MATCHING
BtlObj *BtlFxStart38(void)
{
    BtlObj *o;
    long    pos[3];
    short   n;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    if (g_btl_actor_turn < BTL_PARTY) {
        pos[1] = -FX_38_Y;
    } else {
        pos[1] = FX_38_Y;
    }
    pos[0] = 0;
    pos[2] = FX_38_Z;
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->mark_num = FX_MARK_HEAD;
    o->steps = FX_38_STEPS;
    o->fade = FX_38_FADE;
    o->attr = FX_38_ATTR;
    o->rgb[0] = 0;
    o->rgb[1] = 0;
    o->rgb[2] = 0;
    /* The three walked-to channels go through the one local, the way
       BtlFxStart6E's tail does. */
    n = FX_GREY;
    o->rgb_to[0] = n;
    o->rgb_to[1] = n;
    o->rgb_to[2] = n;
    BtlObjSetTimer(o, FX_38_TIMER);
    return o;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxspell38", BtlFxStart38);
#endif
