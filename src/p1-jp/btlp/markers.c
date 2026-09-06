/* Persona 1 (JP) - where a row of party markers goes.  BTLP only.
 *   0x800AD9AC BtlPlaceMemberMarkers
 *
 * Each marker's screen x comes off the member it belongs to rather than being
 * stored: the object's grid column, halved out of the doubled form the record
 * keeps and taken four pixels a cell. The two bases are the whole point of the
 * second argument - one row of markers sits well left of the other.
 *
 * A member who is not there, is down, or is flagged out of the fight has its
 * entry left alone rather than moved off screen, so a row keeps whatever it
 * last held for the slots that are no longer drawn.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* Markers to a row, and how wide one is. */
#define MARKER_ROW 5

/* The two places a row can start from. */
#define MARKER_X_NEAR (-0xC)
#define MARKER_X_FAR  (-0x2D)

/* Pixels per grid column, once the doubled column has been halved. */
#define MARKER_X_STEP 4

typedef struct {
    /* 0x0 */ short  x;
    /* 0x2 */ u_char pad2[6];
} BtlMarker;                    /* 8 bytes */

extern BtlMarker g_btl_member_marker[];

void BtlPlaceMemberMarkers(int row, int near)
{
    BtlMarker *mark;
    u_char    *key;
    int        off;

    mark = &g_btl_member_marker[row * MARKER_ROW];
    key = &g_btl_actors[0].c.key;
    off = 0;
    do {
        if (near != 0) {
            if (*key != 0
                && *(signed char *)((char *)&g_btl_actors[0].c.status + off)
                       != BTL_STATUS_DOWN
                && (*(u_long *)((char *)&g_btl_actors[0].flags + off)
                    & BTL_ACTOR_OUT) == 0) {
                mark->x = (*(BtlObj **)((char *)&g_btl_actors[0].obj + off))
                              ->col2 / 2 * MARKER_X_STEP + MARKER_X_NEAR;
            }
        } else {
            if (*key != 0
                && *(signed char *)((char *)&g_btl_actors[0].c.status + off)
                       != BTL_STATUS_DOWN
                && (*(u_long *)((char *)&g_btl_actors[0].flags + off)
                    & BTL_ACTOR_OUT) == 0) {
                mark->x = (*(BtlObj **)((char *)&g_btl_actors[0].obj + off))
                              ->col2 / 2 * MARKER_X_STEP + MARKER_X_FAR;
            }
        }
        key += sizeof(BtlActor);
        off += sizeof(BtlActor);
        mark++;
    } while ((int)key < (int)&g_btl_actors[BTL_PARTY].c.key);
}
