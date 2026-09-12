/* Persona 1 (JP) - the move drawn as one record standing behind another.
 * BTLP only.
 *   0x800B8424 BtlFxStart19
 *
 * A start handler out of g_btl_spell_fx, reached by move 0x19 and borrowed
 * outright by 0x1A. Two ordinary effect records are opened on the fighter
 * aimed at and both are brought the same distance toward the camera. The back
 * one is marked, lit and picked out; the front one carries it along on
 * `attached`, so whatever frees the front record takes the back one with it,
 * and the front one is what the table is answered with.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* How far toward the camera both records stand - 16.16, so a unit and a
   half. */
#define FX_19_NEAR 0x180000

/* Neither nought nor FX_MARK_HEAD, so BtlFxStep passes the back record over
   when it goes looking for the head of a chain. */
#define FX_19_BACK_MARK 1

BtlObj *BtlFxStart19(void)
{
    BtlObj *back;
    BtlObj *front;

    back = BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
    back->mark_num = FX_19_BACK_MARK;
    back->z -= FX_19_NEAR;
    back->attr |= BTL_OBJ_PICKED | BTL_OBJ_ATTR_4000;
    front = BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
    front->attached = back;
    front->z -= FX_19_NEAR;
    return front;
}
