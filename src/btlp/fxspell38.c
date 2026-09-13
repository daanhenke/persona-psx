/* Persona 1 (JP) - move 0x38's effect: a record that settles over the far side
 * of the field and lays a sheet across it.  BTLP only.
 *   0x800BA6B8 BtlFxStart38  0x800BA7A0 BtlFxStep38
 *
 * A start handler out of g_btl_spell_fx. One record on the centre line, eighty
 * units past the middle toward whichever side is being aimed at and sixteen
 * above the floor. It is not hidden, so it is drawn from the frame it is taken,
 * but it is static until it has settled; it starts black and is told to walk
 * to half grey.
 *
 * The side is written straight into the position rather than through a local.
 * Taken into a local first, the two reaches for g_btl_fx_def fall into one
 * extended block and gcc shares the address between them; the image
 * materialises the template's address twice, which is what says they are not
 * in the same block.
 *
 * BtlFxStep38 is the move's step handler, run once a frame on each record of
 * the set, which it tells apart by the mark. The head sinks half a unit a frame
 * for as long as its glide lasts, down onto the floor, and then its script is
 * let run. Once the script has finished it lays the five-by-five sheet
 * BtlOpenFxGrid lays over the far side, every cell out of the second script
 * table but wearing the head's own kind and scripts, and each arriving at half
 * the time the scatter table gives it; then the head is walked back to black,
 * and FX_38_HOLD frames later the hit is armed. A cell stays hidden and static
 * until its own timer runs out.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
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

/* How far the head sinks each frame of the glide: sixteen units over its
   thirty-two frames, which puts it on the floor. */
#define FX_38_SINK 0x8000

/* The script table the sheet is drawn from and the motion its cells are put
   on, how long the head takes to walk back to black once the sheet is laid,
   and the phase it is left on once the hit is armed. */
#define FX_38_SHEET_TABLE 1
#define FX_38_CELL_MOTION 2
#define FX_38_HOLD        0x3C
#define FX_38_DONE        0x80

BtlObj *BtlFxStart38(void)
{
    BtlObj *o;
    long    pos[3];
    short   n;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    if (g_btl_actor_turn < BTL_PARTY) {
        pos[1] = -FX_38_Y;
        pos[0] = 0;
        pos[2] = FX_38_Z;
    } else {
        pos[1] = FX_38_Y;
        pos[0] = 0;
        pos[2] = FX_38_Z;
    }
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

void BtlFxStep38(BtlObj *o)
{
    BtlObj *n;
    BtlObj *after;
    long    pos[3];
    long    x;
    long    y_party;
    long    y_enemy;
    int     row;
    int     col;
    int     cell;

    switch (o->phase) {
    case 0:
        switch (o->mark_num) {
        case FX_MARK_HEAD:
            if (o->steps-- > 0) {
                o->z -= FX_38_SINK;
            } else {
                o->attr &= ~BTL_OBJ_STATIC;
                o->phase++;
            }
            break;
        case FX_COPY_MARK:
            if (o->timer == 0) {
                o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
            }
            break;
        }
        break;
    case 1:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            break;
        }
        row = FX_GRID_H - 1;
        cell = FX_GRID_W * FX_GRID_H - 1;
        after = NULL;
        g_btl_fx_def.scripts =
            ((const u_long ***)g_btl_unused_gfx)[FX_38_SHEET_TABLE];
        y_party = FX_GRID_Y_PARTY;
        y_enemy = FX_GRID_Y_ENEMY;
        do {
            col = 0;
            x = FX_GRID_X0;
            do {
                /* The depth is written at the end of both arms: gcc merges the
                   two into one store at the join, ahead of the call's
                   arguments. Written once after the if, it lands among them. */
                if (g_btl_actor_turn < BTL_PARTY) {
                    pos[0] = x;
                    pos[1] = y_enemy;
                    pos[2] = 0;
                } else {
                    pos[0] = x;
                    pos[1] = y_party;
                    pos[2] = 0;
                }
                n = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW,
                                0, pos, FX_OBJ_CD, FX_OBJ_CE);
                n->attr = FX_OBJ_ATTR;
                n->motion = FX_38_CELL_MOTION;
                x += FX_GRID_DX;
                n->kind = o->kind;
                n->scripts = o->scripts;
                n->mark_num = FX_COPY_MARK;
                n->timer = g_btl_fx_cell_order[cell] >> 1;
                col++;
                n->attached = after;
                after = n;
                cell--;
            } while (col < FX_GRID_W);
            y_party -= FX_GRID_DY;
            y_enemy -= FX_GRID_DY;
            row--;
        } while (row >= 0);
        o->rgb_to[0] = 0;
        o->rgb_to[1] = 0;
        o->rgb_to[2] = 0;
        o->timer = FX_38_HOLD;
        o->phase++;
        break;
    case 2:
        if (o->timer != 0) {
            break;
        }
        o->phase = FX_38_DONE;
        BtlArmHitChain();
        break;
    default:
        g_btl_spell_fx[g_btl_fx_move].finish(o);
        break;
    }
}
