/* Persona 1 (JP) - an effect built out of three layers.  BTLP only.
 *   0x800C0578 BtlOpenFxLayers
 *
 * Three records instead of BtlOpenFxObj's one, all standing where the fighter
 * in the given slot stands and chained to each other. The two above take the
 * second of the staged script tables and the bottom one takes the first, so
 * the layers move differently over the same artwork, and each is given a later
 * timer than the one under it - which is what staggers them.
 *
 * The fighter itself is set walking toward the effect's colour before any of
 * them is made.
 *
 * Two of the moves spread their layers sideways as well as in time: they take
 * the tighter of the two timer steps and a shift per layer out of a table of
 * three. Everything else only staggers.
 *
 * The bottom layer is the one answered, so a caller that keeps the result is
 * holding the head of the chain rather than the last record made.

 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* The group effect records come out of, and what one is drawn as. */
#define FX_OBJ_GROUP 2
#define FX_OBJ_DRAW  5

/* The two bytes BtlObjAlloc leaves at +0xCD and +0xCE. */
#define FX_OBJ_CD 0x1D
#define FX_OBJ_CE 0xE

/* Layers, and how fast the fighter walks toward the effect's colour. */
#define FX_LAYERS 3
#define FX_FADE   4

/* What a layer starts as: hidden and static, without a shadow, and with the
   two bits at 0xC0000 the flat drawer reads. */
#define FX_LAYER_ATTR 0x600C0001

/* How far apart the layers are in time, and the two moves that use the
   tighter step and spread them sideways as well. */
#define FX_STEP_WIDE   8
#define FX_STEP_TIGHT  2
#define FX_MOVE_SPREAD_A 2
#define FX_MOVE_SPREAD_B 5

extern BtlObjDef g_btl_fx_def;

/* How far each layer of a spread move stands from the middle, in pixels. */
extern signed char g_btl_fx_shift[];

/* The colour a fighter is put on while an effect stands over it. */
extern u_char g_btl_tint_fx_r;
extern u_char g_btl_tint_fx_g;
extern u_char g_btl_tint_fx_b;

extern u_char g_btl_fx_move;
extern u_char *g_btl_unused_gfx;

BtlObj *BtlOpenFxLayers(int slot, int timer)
{
    BtlObj *ob;
    BtlObj *after;
    BtlObj *head;
    long    pos[3];
    int     shift;
    int     layer;

    g_btl_actors[slot].obj->rgb_to[0] = g_btl_tint_fx_r;
    g_btl_actors[slot].obj->rgb_to[1] = g_btl_tint_fx_g;
    g_btl_actors[slot].obj->rgb_to[2] = g_btl_tint_fx_b;
    g_btl_actors[slot].obj->fade = FX_FADE;

    layer = FX_LAYERS - 1;
    after = 0;
    do {
        ob = g_btl_actors[slot].obj;
        pos[0] = ob->x;
        pos[1] = ob->y;
        pos[2] = 0;
        if (layer != 0) {
            g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[1];
        } else {
            g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
        }
        /* The one local carries the fighter's object on the way in and the
           new record on the way out - two unrelated things, and the image
           keeps them in the one register. Given a variable of its own the
           attribute word is built before the call's answer is moved out of
           v0, and the two end up the wrong way round. */
        ob = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                         pos, FX_OBJ_CD, FX_OBJ_CE);
        ob->attr = FX_LAYER_ATTR;
        ob->mark_num = layer;
        ob->attached = after;
        if (g_btl_fx_move == FX_MOVE_SPREAD_A
            || g_btl_fx_move == FX_MOVE_SPREAD_B) {
            /* The shift is taken before the timer is written, which is what
               fills the load's delay slot with the timer's own sum. */
            shift = g_btl_fx_shift[layer];
            ob->timer = layer * FX_STEP_TIGHT + timer;
            ob->shift_x = shift << 16;
        } else {
            ob->timer = layer * FX_STEP_WIDE + timer;
        }
        if (layer == 0) {
            head = ob;
        }
        after = ob;
        layer--;
    } while (layer >= 0);
    return head;
}
