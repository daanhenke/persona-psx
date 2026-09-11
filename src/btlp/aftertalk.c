/* Persona 1 (JP) - pulling the party forward once a negotiation is over.
 * BTLP only.
 *   0x8009BBC0 BtlAfterTalk
 *
 * A contact leaves the front rows empty - whoever stepped out to talk is back
 * in the grid but the demon that stood in front of them is gone - so the whole
 * party is walked forward by however many rows are empty in front of it.
 *
 * The grid is found, cleared outright, and written again from where the
 * members actually stand, which is what keeps it in step with the objects
 * rather than the other way round. Each member is set gliding rather than put
 * down: the distance is divided by the sixteen frames the walk takes and left
 * on the object as its step.
 *
 * A row is occupied if any of its five cells is. Nothing is done at all when
 * the front row is already occupied, because the walk is then nought rows.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>

/* What a member walking forward is put on, and how long it takes. */
#define TALK_WALK_MOTION 0xD
#define TALK_WALK_FRAMES 0x10

#ifdef NON_MATCHING
void BtlAfterTalk(void)
{
    BtlObj *o;
    u_char *cell;
    u_char  empty;
    int     rows;
    int     col;
    int     at;
    int     taken;
    int     slot;

    rows  = 0;
    taken = 0;
    do {
        for (col = 0; col < GRID_W; col++) {
            if (g_btl_formation[rows * GRID_W + col] != CELL_EMPTY) {
                taken = 1;
                break;
            }
        }
        if (taken) {
            break;
        }
        rows++;
    } while (rows < GRID_H);

    /* Cleared back to front through a pointer of its own; indexed, the store
       works the address out afresh every turn. */
    empty = CELL_EMPTY;
    at    = GRID_CELLS - 1;
    cell  = &g_btl_formation[GRID_CELLS - 1];
    do {
        *cell = empty;
        at--;
        cell--;
    } while (at >= 0);

    slot = 0;
    do {
        if (g_btl_actors[slot].c.key != 0
            && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
            && !(g_btl_actors[slot].flags & BTL_ACTOR_OUT)) {
            g_btl_actors[slot].obj->step_y =
                -(rows * (PLACE_ROW_H * PLACE_FIXED)) / TALK_WALK_FRAMES;
            g_btl_actors[slot].obj->row   -= rows;
            g_btl_actors[slot].obj->motion = TALK_WALK_MOTION;
            g_btl_actors[slot].obj->unkBC  = TALK_WALK_FRAMES;

            o = g_btl_actors[slot].obj;
            g_btl_formation[o->row * GRID_W + (o->col2 >> 1)] = slot;
        }
        slot++;
    } while (slot < BTL_PARTY);
}
#else
INCLUDE_ASM("btlp/nonmatchings/aftertalk", BtlAfterTalk);
#endif
