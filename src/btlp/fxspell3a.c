/* Persona 1 (JP) - the move whose one record stands over the field at a depth
 * its own id picks.  BTLP only.
 *   0x800BADA4 BtlFxStart3A  0x800BAE5C BtlFxStep3A  0x800BB178 BtlFxStartHigh
 *
 * A start handler out of g_btl_spell_fx, shared by moves 0x3A and 0x3E. The
 * position is not built on the stack: the two moves stand their record at the
 * same place except for how far toward the camera it is, so the handler writes
 * that one word into the shared g_btl_fx_lift and hands the whole vector over.
 * Eighty-eight units for 0x3A and sixty for its partner.
 *
 * The record is drawn and stepped from the frame it is taken - neither hidden
 * nor static - and starts black, walking to half grey three steps a frame.
 *
 * BtlFxStep3A is the step handler for 0x3A and 0x3E. The head floats the whole
 * time, its height read out of g_btl_fx_hover a quarter as fast as it ages.
 * Once it is grey, a record of move 0x3A's own kind is put on the second of its
 * scripts and held FX_3A_RISE frames, and any other has its script let run;
 * after that it is put on its third script, walked back to black, and a record
 * is opened on every fighter the move reaches - each of them marked as the
 * rest of the set, of the head's kind, and on one of the staged scripts. Once
 * the head is black it is hidden and the hit is armed the way BtlArmHitChain
 * arms it, on the fighter aimed at. The records on the fighters wait out their
 * timers, are shown, and free themselves once their script has run.
 *
 * BtlFxStartHigh is BtlFxStart3A's record standing on g_btl_fx_high instead,
 * with no depth to choose. Nothing in the overlay reaches it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* Which move takes the deeper of the two places, and what the two are. */
#define FX_3A_MOVE 0x3A
#define FX_3A_DEEP 0x580000
#define FX_3A_NEAR 0x3C0000

/* What the record starts as, and how fast it takes on colour. */
#define FX_3A_ATTR BTL_OBJ_NO_SHADOW
#define FX_3A_FADE 3

/* The head's two later scripts, and how long move 0x3A's holds on the first
   of them; which of the staged scripts the records on the fighters are put on
   for move 0x3A and for any other, and the motion they are given; and the
   phase the head is left on once the hit is armed. */
#define FX_3A_RISE_SCRIPT  1
#define FX_3A_BURST_SCRIPT 2
#define FX_3A_RISE         0x1E
#define FX_3A_OWN_GFX      3
#define FX_3A_OTHER_GFX    1
#define FX_3A_MOTION       2
#define FX_3A_DONE         0x80

/* How high a floating effect stands, one entry per four frames of its age, in
   whole units. */
extern long g_btl_fx_hover[];

BtlObj *BtlFxStart3A(void)
{
    BtlObj *o;
    short   n;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    /* The depth is chosen inside the store, not through a local and not in
       two stores of its own: either of those puts the address of the vector
       somewhere the image does not have it. */
    g_btl_fx_lift[2] =
        g_btl_fx_move == FX_3A_MOVE ? -FX_3A_DEEP : -FX_3A_NEAR;
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                    g_btl_fx_lift, FX_OBJ_CD, FX_OBJ_CE);
    o->attr = FX_3A_ATTR;
    o->mark_num = FX_MARK_HEAD;
    o->fade = FX_3A_FADE;
    /* The walked-to colour goes through the one local, and the colour it
       starts on after it - the order the image writes them in. */
    n = FX_GREY;
    o->rgb_to[0] = n;
    o->rgb_to[1] = n;
    o->rgb_to[2] = n;
    o->rgb[0] = 0;
    o->rgb[1] = 0;
    o->rgb[2] = 0;
    return o;
}

/* 97.94%. One load is out of place. In the last phase the image reads
   g_btl_fx_target straight after the move it looks the group up by, ahead of
   the shift that indexes the table, and keeps it in a0 through the hit
   stores; this reads it after the group, just ahead of the fighter in turn.
   Where `slot` is taken - first, before or after the group, among the hit
   stores, chained into the hit slot's store - and its width do not move the
   load, and neither does the move taken into a local; dropping the group's
   cast, storing the attribute or the phase later, or a local for the bit each
   leave it further off. The copies' kind written ahead of their motion and
   the target held in `slot` are what took it from 91.78%. */
#ifdef NON_MATCHING
void BtlFxStep3A(BtlObj *o)
{
    BtlObj *n;
    int     slot;

    if (o->mark_num == FX_MARK_HEAD) {
        switch (o->phase) {
        case 0:
            if (o->rgb[0] != FX_GREY) {
                break;
            }
            if (o->kind == FX_3A_MOVE) {
                BtlObjSetScript(o, (BtlSeqStep *)o->scripts[FX_3A_RISE_SCRIPT]);
                o->timer = FX_3A_RISE;
            } else {
                o->attr &= ~BTL_OBJ_STATIC;
            }
            o->phase++;
            break;
        case 1:
            if (o->timer != 0) {
                break;
            }
            if (o->kind == FX_3A_MOVE) {
                BtlObjSetScript(o, (BtlSeqStep *)o->scripts[FX_3A_BURST_SCRIPT]);
            }
            o->rgb_to[0] = 0;
            o->rgb_to[1] = 0;
            o->rgb_to[2] = 0;
            for (n = BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
                 n != NULL; n = n->attached) {
                n->mark_num = FX_MARK_REST;
                n->kind = o->kind;
                n->motion = FX_3A_MOTION;
                BtlObjSetScript(n, o->kind == FX_3A_MOVE
                    ? (BtlSeqStep *)((const u_long ***)g_btl_unused_gfx)[FX_3A_OWN_GFX]
                    : (BtlSeqStep *)((const u_long ***)g_btl_unused_gfx)[FX_3A_OTHER_GFX]);
            }
            o->phase++;
            break;
        case 2:
            if (o->rgb[0] != 0) {
                break;
            }
            o->phase = FX_3A_DONE;
            o->attr |= BTL_OBJ_HIDDEN;
            o->children = (u_char)g_btl_spell_fx[g_btl_fx_move].group;
            slot = g_btl_fx_target;
            g_btl_hits_left = 9;
            g_btl_hit_walk = -1;
            g_btl_hit_mask = 1;
            g_btl_hit_slot = slot;
            g_btl_actors[g_btl_actor_turn].targets &= ~(1 << slot);
            break;
        default:
            BtlFxFinish37(o);
            break;
        }
        o->z = (g_btl_fx_hover[(o->age >> BTL_HOVER_SHIFT) & (BTL_HOVER_ENTRIES - 1)]
                << BTL_HOVER_FIXED) + o->z2;
    } else {
        switch (o->phase) {
        case 0:
            if (o->timer == 0) {
                o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
                o->phase++;
            }
            break;
        case 1:
            if ((o->attr & BTL_OBJ_ANIMATING) == 0) {
                BtlObjFree(o);
            }
            break;
        }
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxspell3a", BtlFxStep3A);
#endif

BtlObj *BtlFxStartHigh(void)
{
    BtlObj *o;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                    g_btl_fx_high, FX_OBJ_CD, FX_OBJ_CE);
    o->attr = FX_3A_ATTR;
    o->mark_num = FX_MARK_HEAD;
    o->fade = FX_3A_FADE;
    o->rgb_to[0] = FX_GREY;
    o->rgb_to[1] = FX_GREY;
    o->rgb_to[2] = FX_GREY;
    o->rgb[0] = 0;
    o->rgb[1] = 0;
    o->rgb[2] = 0;
    return o;
}
