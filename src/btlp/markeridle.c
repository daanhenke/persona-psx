/* Persona 1 (JP) - the two questions asked of the party's markers.  BTLP only.
 *   0x800ACB3C BtlMarkersIdle  0x800ACB9C BtlMarkersHidden
 *
 * Both are the same shape: walk the six marker chains, or the five words that
 * say whether a marker is up, and answer one only if every one of them agrees.
 * The round leans on them to know when it may go on - a stage that has just put
 * the markers up or taken them down turns the frame over until one of these
 * says the motion is finished.
 *
 * BtlMarkersIdle asks each chain whether it is on motion zero, which is what
 * BtlObjChainAtMotion answers for the whole chain rather than the head alone;
 * BtlMarkersHidden only reads the flags, so it is a leaf and takes no frame.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* Chains there are, and flags there are - a member has a marker each and the
   chains carry one more than that. */
#define MARKER_CHAINS 6
#define MARKER_FLAGS  5

/* The motion a marker stands on when it is not doing anything. */
#define MARKER_IDLE 0

extern int g_btl_marker_shown[];

int BtlMarkersIdle(void)
{
    int i;

    i = 0;
    do {
        if (BtlObjChainAtMotion(g_btl_marker_obj[i], MARKER_IDLE) != 0) {
            i++;
        } else {
            return 0;
        }
    } while (i < MARKER_CHAINS);
    return 1;
}

int BtlMarkersHidden(void)
{
    int *shown;
    int  i;

    i     = 0;
    shown = g_btl_marker_shown;
    do {
        if (*shown != 0) {
            return 0;
        }
        i++;
        shown++;
    } while (i < MARKER_FLAGS);
    return 1;
}
