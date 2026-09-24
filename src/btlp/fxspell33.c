/* Persona 1 (JP) - the move whose record is faced squarely at the camera.
 * BTLP only.
 *   0x800B9DF0 BtlFxStart33  0x800B9EF0 BtlFxStep33
 *
 * A start handler out of g_btl_spell_fx, and BtlFxStart6E written out again
 * with one difference: where 0x6E always stands its record a hundred units
 * above the middle of the field, this one picks the side. Move 0x56 puts it
 * over the side that is acting and everything else over the side being aimed
 * at, which is the only thing the two arms of the outer test differ in.
 *
 * As in 0x6E the record is given no scale at all - its script grows it - and
 * the three angles it is drawn through are the camera's own two plus the
 * distance the opening pulled the camera back by, which keeps it flat against
 * the view however the field is turned.
 *
 * BtlFxStep33 is the step handler, run on each record of the set and told
 * which one it is by the mark. The head turns on its own axis the whole time.
 * It grows by half its size a frame until its scale reaches FX_33_FULL, and
 * then opens a second record on its own position out of the staged artwork's
 * second script table, wearing the head's kind and scripts and walking up from
 * black; FX_33_HOLD frames later the head walks back to black, and once that
 * has had FX_33_DIM frames it is hidden and the hit is armed.
 *
 * The second record rises a unit a frame until it stands thirty-two units
 * toward the camera, lets its script run, and once that has finished lays
 * BtlOpenFxGrid's sheet out of the third table - every cell on the move's own
 * kind and marked FX_33_SHEET_MARK - then walks to black and frees itself. A
 * cell of the sheet is shown once its own timer runs out.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How far above the middle of the field the record stands - 16.16, so a
   hundred whole units, the same as move 0x6E's. */
#define FX_33_UP 0x640000

/* The one move of the set that stands its record over its own side instead of
   the other one. */
#define FX_33_SELF 0x56

/* What the record starts as - the same as move 0x6E's: neither hidden nor
   static, and carrying the bit the whitening also sets. */
#define FX_33_ATTR (BTL_OBJ_ATTR_4000 | 0x5)

/* How fast the head grows and how big it gets, and how far it turns a frame. */
#define FX_33_GROW 0x80
#define FX_33_FULL 0x1F40
#define FX_33_SPIN 0x20

/* The second record: the table it comes out of, what it starts as, how fast
   and how far it rises, and the motion it and the sheet are put on. */
#define FX_33_REST_TABLE 1
#define FX_33_REST_ATTR  (BTL_OBJ_STATIC | BTL_OBJ_NO_SHADOW)
#define FX_33_RISE       (-0x10000)
#define FX_33_TOP        (-0x200000)
#define FX_33_MOTION     2

/* The sheet: the table it comes out of and the mark its cells carry. */
#define FX_33_SHEET_TABLE 2
#define FX_33_SHEET_MARK  0x12

/* How fast the colours walk, how long the head holds before it dims and how
   long a dimming takes, and the phase the head is left on once the hit is
   armed. */
#define FX_33_FADE 4
#define FX_33_HOLD 0x3C
#define FX_33_DIM  0x1E
#define FX_33_DONE 0x80

BtlObj *BtlFxStart33(void)
{
    BtlObj *o;
    long    pos[3];
    short   n;

    pos[0] = 0;
    pos[2] = 0;
    /* Both sides are picked inside the store, and the two are spelled with
       opposite tests although they mean the same pair of places: written as
       an if of its own each arm costs three words, and written with the same
       test both ways one of the two branches comes out inverted. */
    if (g_btl_fx_move == FX_33_SELF) {
        pos[1] = g_btl_actor_turn >= BTL_PARTY ? -FX_33_UP : FX_33_UP;
    } else {
        pos[1] = g_btl_actor_turn < BTL_PARTY ? -FX_33_UP : FX_33_UP;
    }
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->attr = FX_33_ATTR;
    o->mark_num = FX_MARK_HEAD;
    o->scale_x = 0;
    o->scale_y = 0;
    /* The three angles go through the one local, the way 0x6E's tail does. */
    n = g_btl_cam_rot.vx;
    o->rot.vx = n;
    n = g_btl_cam_rot.vy;
    o->rot.vy = n;
    n = g_btl_intro_dist[0];
    o->rot.vz = n;
    return o;
}

void BtlFxStep33(BtlObj *o)
{
    BtlObj *n;

    switch (o->mark_num) {
    case FX_MARK_HEAD:
        switch (o->phase) {
        case 0:
            o->scale_x += FX_33_GROW;
            o->scale_y += FX_33_GROW;
            if (o->scale_x >= FX_33_FULL) {
                o->phase++;
            }
            break;
        case 1:
            g_btl_fx_def.scripts =
                ((const u_long ***)g_btl_unused_gfx)[FX_33_REST_TABLE];
            n = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                            &o->x, FX_OBJ_CD, FX_OBJ_CE);
            n->attr = FX_33_REST_ATTR;
            n->kind = o->kind;
            n->scripts = o->scripts;
            n->mark_num = FX_MARK_REST;
            n->step_z = FX_33_RISE;
            n->fade = FX_33_FADE;
            n->motion = FX_33_MOTION;
            n->rgb_to[0] = FX_GREY;
            n->rgb_to[1] = FX_GREY;
            n->rgb_to[2] = FX_GREY;
            n->rgb[0] = 0;
            n->rgb[1] = 0;
            n->rgb[2] = 0;
            o->timer = FX_33_HOLD;
            o->phase++;
            break;
        case 2:
            if (o->timer != 0) {
                break;
            }
            o->fade = FX_33_FADE;
            o->rgb_to[0] = 0;
            o->rgb_to[1] = 0;
            o->rgb_to[2] = 0;
            o->timer = FX_33_DIM;
            o->phase++;
            break;
        case 3:
            if (o->timer != 0) {
                break;
            }
            o->phase = FX_33_DONE;
            o->attr |= BTL_OBJ_HIDDEN;
            BtlArmHitChain();
            o->children = 0;
            break;
        default:
            BtlFxFinish01(o);
            break;
        }
        o->rot.vz += FX_33_SPIN;
        break;
    case FX_MARK_REST:
        switch (o->phase) {
        case 0:
            o->z += o->step_z;
            if (o->z <= FX_33_TOP) {
                o->z = FX_33_TOP;
                o->attr &= ~BTL_OBJ_STATIC;
                o->phase++;
            }
            break;
        case 1:
            if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
                break;
            }
            n = BtlOpenFxGrid(FX_33_SHEET_TABLE);
            BtlObjSetKind(n, o->kind);
            BtlObjSetMarkNum(n, FX_33_SHEET_MARK);
            BtlObjSetMotion(n, FX_33_MOTION);
            o->timer = FX_33_DIM;
            o->phase++;
            break;
        case 2:
            if (o->timer != 0) {
                break;
            }
            o->rgb_to[0] = 0;
            o->rgb_to[1] = 0;
            o->rgb_to[2] = 0;
            o->timer = FX_33_DIM;
            o->phase++;
            break;
        case 3:
            if (o->timer != 0) {
                break;
            }
            BtlObjFree(o);
            break;
        }
        break;
    case FX_33_SHEET_MARK:
        if (o->phase == 0 && o->timer == 0) {
            o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
            o->phase++;
        }
        break;
    }
}
