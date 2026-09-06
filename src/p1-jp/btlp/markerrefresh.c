/* Persona 1 (JP) - starting the party markers moving.  BTLP only.
 *   0x800A82CC BtlRefreshMarkers
 *
 * The markers stand over the party, so anything that moves the party moves
 * them: the whole table of positions is worked out again and then each of the
 * six objects is set going. The delays are what makes them arrive one after
 * another rather than snapping across together.
 */
#include <types.h>
#include <persona/btlp/object.h>

/* Marker objects, and the motion they travel in. */
#define MARKERS       6
#define MARKER_MOTION 3

extern BtlObj      *g_btl_marker_obj[];
extern const u_char g_btl_marker_delay[];

extern void BtlBuildMarkers(void);
extern void BtlObjSetTimer(BtlObj *obj, short timer);
extern void BtlObjSetMotion(BtlObj *obj, u_char motion);

void BtlRefreshMarkers(void)
{
    BtlObj **slot;
    int      i;

    BtlBuildMarkers();
    i = 0;
    slot = g_btl_marker_obj;
    do {
        BtlObjSetTimer(*slot, g_btl_marker_delay[i] * 2);
        i++;
        BtlObjSetMotion(*slot, MARKER_MOTION);
        slot++;
    } while (i < MARKERS);
}
