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
 *
 * The record is read by slot rather than walked by a byte offset. Written the
 * other way gcc turns each field into a walking pointer of its own and the
 * routine comes out eight instructions short; left as an index it makes the
 * one offset it needs and reaches every field through it, which is what the
 * original does.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>

/* Markers to a row, and how wide one is. */
#define MARKER_ROW 5

/* The two places a row can start from. */
#define MARKER_X_NEAR (-0xC)
#define MARKER_X_FAR  (-0x2D)

/* Pixels per grid column, once the doubled column has been halved. */
#define MARKER_X_STEP 4

void BtlPlaceMemberMarkers(int row, int near)
{
    BtlMarker *mark;
    int        slot;

    mark = &g_btl_member_marker[row * MARKER_ROW];
    slot = 0;
    do {
        if (near != 0) {
            if (g_btl_actors[slot].c.key != 0
                && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
                && (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0) {
                mark->x = g_btl_actors[slot].obj->col2 / 2 * MARKER_X_STEP + MARKER_X_NEAR;
            }
        } else {
            if (g_btl_actors[slot].c.key != 0
                && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
                && (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0) {
                mark->x = g_btl_actors[slot].obj->col2 / 2 * MARKER_X_STEP + MARKER_X_FAR;
            }
        }
        slot++;
        mark++;
    } while (slot < BTL_PARTY);
}

