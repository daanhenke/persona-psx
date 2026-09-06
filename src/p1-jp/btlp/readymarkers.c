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
#include <types.h>
#include <persona/btlp/actor.h>

/* The marker is up. */
#define MARKER_UP 3

/* The two ailments with markers of their own, and what everything else takes. */
#define MARKER_AIL_A 0x15
#define MARKER_AIL_B 0x16
#define MARKER_KIND_A 0
#define MARKER_KIND_B 1
#define MARKER_OTHER  6

extern int  BtlStatusStops(const BtlActor *a);
extern void BtlShowMarker(int slot, int on, int kind);

void BtlShowReadyMarkers(void)
{
    int     slot;
    int     off;
    int     kind;


    off = 0;
    slot = 0;
    do {
        if (*((u_char *)&g_btl_actors[0].c.key + off) == 0) {
            goto clear;
        }
        if (*(signed char *)((char *)&g_btl_actors[0].c.status + off)
            == BTL_STATUS_DOWN) {
            goto clear;
        }
        if ((*(u_long *)((char *)&g_btl_actors[0].flags + off) & BTL_ACTOR_OUT)
            != 0) {
            goto clear;
        }
        if (BtlStatusStops((BtlActor *)((char *)g_btl_actors + off)) != 0) {
            goto clear;
        }
        if (*((u_char *)&g_btl_actors[0].marker + off) == MARKER_UP) {
            goto step;
        }
        *((u_char *)&g_btl_actors[0].marker + off) = MARKER_UP;
        switch (*(signed char *)((char *)&g_btl_actors[0].c.status + off)) {
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
        *((u_char *)&g_btl_actors[0].marker + off) = 0;
    step:
        slot++;
        off += sizeof(BtlActor);
    } while (slot < BTL_PARTY);
}
