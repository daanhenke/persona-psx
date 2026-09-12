/* Persona 1 (JP) - the motion that shows a record for as long as its timer.
 * BTLP only.
 *   0x800AD8E8 BtlObjMotion0C
 *
 * A motion handler out of g_btl_obj_motion, and the shortest of them: while the
 * record's timer is still running it is drawn, and the frame the timer reaches
 * zero it is hidden again and the motion ends. Nothing counts the timer down in
 * here - the frame pass does that for every record after its handler - so this
 * is only the showing, not the timing.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>

void BtlObjMotion0C(BtlObj *obj)
{
    if (obj->timer != 0) {
        obj->attr &= ~BTL_OBJ_HIDDEN;
    } else {
        obj->motion = 0;
        obj->phase  = 0;
        obj->attr  |= BTL_OBJ_HIDDEN;
    }
}
