/* Persona 1 (JP) - the short-lived things the battle throws on screen.
 *   BTLP @ 0x8008AAAC BtlTickPopup
 *
 * This is the tick handler for the first display-object group, and it knows
 * three kinds. One shakes: it walks g_btl_shake a frame at a time, indexed by
 * its own age, and the table damps out - five frames four units one way and
 * five back, then four of three each way, three of two, and so on. One drifts,
 * a whole unit a frame, until the timer it was given runs out. The third only
 * waits. All three end the same way, holding for half a second and freeing
 * themselves, which is why the tail is shared.
 *
 * The hold is thirty frames, or fifteen when the battle is running at half the
 * field's rate - that setting halves every frame count in the overlay so the
 * motion keeps its wall-clock speed.
 */
#include <types.h>
#include <persona/btlp/object.h>

/* Kinds, out of the object's kind field. */
#define POPUP_SHAKE 0x0D
#define POPUP_DRIFT 0x12
#define POPUP_WAIT  0x15

/* Half a second, and what it becomes at half the frame rate. */
#define POPUP_HOLD      30
#define POPUP_HOLD_HALF 15

/* One whole unit of the 16.16 displacement. */
#define POPUP_STEP 0x10000

extern short  g_btl_shake[];
extern u_char g_btl_half_rate;

extern int BtlObjFree(BtlObj *obj);

void BtlTickPopup(BtlObj *obj)
{
    u_short kind;
    short   hold;

    kind = obj->kind;
    switch (kind) {
    case POPUP_SHAKE:
        switch (obj->phase) {
        case 0:
            if (g_btl_shake[obj->age] != 0) {
                obj->shift = g_btl_shake[obj->age] * POPUP_STEP + obj->shift;
                return;
            }
            hold = POPUP_HOLD;
            if (g_btl_half_rate != 0) {
                hold = POPUP_HOLD_HALF;
            }
            obj->timer = hold;
            obj->phase++;
            return;
        case 1:
            break;
        default:
            return;
        }
        break;
    case POPUP_DRIFT:
        switch (obj->phase) {
        case 0:
            obj->shift = obj->shift + POPUP_STEP;
            if (obj->timer != 0) {
                return;
            }
            obj->timer = POPUP_HOLD;
            obj->phase++;
            break;
        case 1:
            if (obj->timer != 0) {
                return;
            }
            /* Freed here and again by the shared tail below, which finds the
               timer still zero. That is what the original does. */
            BtlObjFree(obj);
            break;
        }
        break;
    case POPUP_WAIT:
        break;
    default:
        return;
    }
    if (obj->timer == 0) {
        BtlObjFree(obj);
    }
}
