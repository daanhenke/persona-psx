/* Persona 1 (JP) - the move that throws sparks round the middle of the field.
 * BTLP only.
 *   0x800B8B70 BtlFxStart27  0x800B8C30 BtlFxStep27
 *
 * The start handler opens one record over the acting side and puts the page
 * its artwork is drawn from into subtractive blending, so what the sparks are
 * drawn over darkens rather than lightens.
 *
 * The step handler is the one that does the work. Every fourth frame it opens
 * a spark of its own out of the record's second script, on one of the eight
 * cells around the middle of the field, and hangs it off the record so the
 * next frame can finish setting it up. When the record's own timer runs out it
 * starts whitening, and the hit is armed the frame the white arrives.
 *
 * The sparks carry the second mark, and each frees itself once its script has
 * played out.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* Where the record the start handler opens stands - 16.16, a hundred units
   above the middle of the field or a hundred below. */
#define FX_27_OFF 0x640000

/* How long it stands there, and the two bits it is given on top of what
   BtlObjAlloc left. */
#define FX_27_FRAMES 0x3C
#define FX_27_ATTR   3

/* The upload slot the sparks are drawn from. */
#define FX_27_SLOT 0x1D

/* One spark every fourth frame, on one of eight cells. */
#define FX_27_EVERY 3
#define FX_27_CELLS 7

/* What a cell's column and row are worth: the columns are a quarter of the
   sixty units between them, the rows a half of the forty. */
#define FX_27_COL_W 15
#define FX_27_ROW_H 20

/* Where the grid's own corner sits on each side. */
#define FX_27_LEFT 0x3C
#define FX_27_HIGH 0x8C
#define FX_27_LOW  0x3C

/* What the sparks carry, and what the record walks to before the hit. */
#define FX_27_SPARK_MOTION 2
#define FX_27_LIT          0xFF
#define FX_27_WHITE        0x00FF00FF
#define FX_27_FADE         4
#define FX_27_DONE         0x80

BtlObj *BtlFxStart27(void)
{
    BtlObj *o;
    long    pos[3];

    if (g_btl_actor_turn < BTL_PARTY) {
        pos[0] = 0;
        pos[1] = -FX_27_OFF;
        pos[2] = 0;
    } else {
        pos[0] = 0;
        pos[1] = FX_27_OFF;
        pos[2] = 0;
    }
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->mark_num = FX_MARK_HEAD;
    o->timer    = FX_27_FRAMES;
    o->attr    |= FX_27_ATTR;
    g_btl_tpage[FX_27_SLOT] =
        (g_btl_tpage[FX_27_SLOT] & ~BTL_TPAGE_BLEND) | BTL_TPAGE_SUBTRACT;
    return o;
}

/* 92.40%: eight bytes of frame this does not need, the side test kept in a
   register where the image works it out twice, and the dead first position's
   own store lifted above the branch instead of landing at the label the arms
   meet at. */
#ifdef NON_MATCHING
void BtlFxStep27(BtlObj *o)
{
    BtlObj *spark;
    long    pos[3];
    int     row;

    switch (o->mark_num) {
    case FX_MARK_HEAD:
        switch (o->phase) {
        case 0:
            if ((g_btl_tick & FX_27_EVERY) != 0) {
                break;
            }
            g_btl_fx_def.scripts = (const u_long **)o->scripts[1];
            pos[0] = 0;
            pos[1] = (g_btl_actor_turn < BTL_PARTY) ? -FX_27_OFF : FX_27_OFF;
            pos[2] = 0;
            pos[0] = (g_btl_fx_ring_cells[(o->steps & FX_27_CELLS) * 2]
                          * FX_27_COL_W - FX_27_LEFT) << 16;
            row = g_btl_fx_ring_cells[(o->steps & FX_27_CELLS) * 2 + 1]
                      * FX_27_ROW_H;
            pos[1] = ((g_btl_actor_turn < BTL_PARTY) ? row - FX_27_HIGH
                                                     : row + FX_27_LOW) << 16;
            pos[2] = 0;
            spark = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                                pos, FX_OBJ_CD, FX_OBJ_CE);
            o->unk54 = (long)spark;
            spark->kind = o->kind;
            ((BtlObj *)o->unk54)->mark_num = FX_MARK_REST;
            ((BtlObj *)o->unk54)->motion = FX_27_SPARK_MOTION;
            o->steps++;
            if (o->timer != 0) {
                break;
            }
            o->rgb_to[0] = FX_27_LIT;
            o->rgb_to[1] = FX_27_LIT;
            o->rgb_to[2] = FX_27_LIT;
            o->fade = FX_27_FADE;
            o->phase++;
            break;
        case 1:
            if (*(u_long *)&o->rgb[0] != FX_27_WHITE) {
                break;
            }
            if (o->rgb[2] != FX_27_LIT) {
                break;
            }
            BtlArmHitChain();
            o->phase = FX_27_DONE;
            o->children = (u_char)g_btl_spell_fx[g_btl_fx_move].group;
            break;
        default:
            BtlFxFinish01(o);
            break;
        }
        break;
    case FX_MARK_REST:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            break;
        }
        BtlObjFree(o);
        break;
    default:
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxspell27", BtlFxStep27);
#endif
