/* Persona 1 (JP) - keeping the party's markers up to date.  BTLP only.
 *   0x80093A84 BtlShowReadyMarkers
 *
 * Run once a frame. A member who is absent, downed, out of the fight, or held
 * by an ailment that stops them acting has their marker taken away; everyone
 * else has one put up. The byte in the actor record is what stops the marker
 * being rebuilt every frame - it only goes up on the frame it changes.
 *
 * Two ailments get markers of their own and everything else takes the sixth,
 * so the marker says not just that a member can act but roughly why they
 * cannot.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/status.h>

/* The marker is up. */
#define MARKER_UP 3

/* The two ailments with markers of their own, and what everything else takes. */
#define MARKER_AIL_A 0x15
#define MARKER_AIL_B 0x16
#define MARKER_KIND_A 0
#define MARKER_KIND_B 1
#define MARKER_OTHER  6

void BtlShowReadyMarkers(void)
{
    int     slot;
    int     up;
    int     kind;
    u_char *mark;

    /* Three things are set up before the walk starts, and this order is the
       one the routine was written in: the marker code is taken into a local
       of its own rather than left as the constant it is, which is what keeps
       it out of the loop's preheader - hoisted, it would be set up after the
       marker pointer instead of before it.

       Only the marker is walked by a pointer; everything else is read out of
       the record by slot, and the compiler makes the one byte offset it
       needs for those. */
    slot = 0;
    up = MARKER_UP;
    mark = &g_btl_actors[0].marker;
    do {
        if (g_btl_actors[slot].c.key == 0) {
            goto clear;
        }
        if ((signed char)g_btl_actors[slot].c.status == BTL_STATUS_DOWN) {
            goto clear;
        }
        if ((g_btl_actors[slot].flags & BTL_ACTOR_OUT) != 0) {
            goto clear;
        }
        if (BtlStatusStops(&g_btl_actors[slot]) != 0) {
            goto clear;
        }
        if (*mark == up) {
            goto step;
        }
        *mark = up;
        switch ((signed char)g_btl_actors[slot].c.status) {
        case MARKER_AIL_A:
            kind = MARKER_KIND_A;
            break;
        case MARKER_AIL_B:
            kind = MARKER_KIND_B;
            break;
        default:
            kind = MARKER_OTHER;
            break;
        }
        BtlShowMarker(slot, 1, kind);
        goto step;
    clear:
        *mark = 0;
    step:
        mark += sizeof(BtlActor);
        slot++;
    } while (slot < BTL_PARTY);
}

