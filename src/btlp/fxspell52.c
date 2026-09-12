/* Persona 1 (JP) - the move whose effect stands over the side that is acting.
 * BTLP only.
 *   0x800BC708 BtlFxStart52
 *
 * A start handler out of g_btl_spell_fx. It is the only one that opens its
 * record by hand rather than through one of the three openers, because it
 * wants a position of its own rather than a fighter's: the record stands on
 * the middle of the field, a hundred units above it or a hundred below
 * depending on which side is acting. The template is filled in from the
 * staged artwork's first script table the same way the openers do it.
 *
 * The position is written out in full in both arms rather than once behind a
 * chosen offset, which is what puts the three stores at the label the arms
 * meet at instead of ahead of the test.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How far off the middle of the field the record stands - 16.16, so a hundred
   whole units. The party's side is the negative one. */
#define FX_52_OFF 0x640000

BtlObj *BtlFxStart52(void)
{
    BtlObj *o;
    long    pos[3];

    if (g_btl_actor_turn < BTL_PARTY) {
        pos[0] = 0;
        pos[1] = -FX_52_OFF;
        pos[2] = 0;
    } else {
        pos[0] = 0;
        pos[1] = FX_52_OFF;
        pos[2] = 0;
    }
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->mark_num = FX_MARK_HEAD;
    o->attr |= FX_OBJ_ATTR;
    return o;
}
