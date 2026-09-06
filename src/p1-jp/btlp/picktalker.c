/* Persona 1 (JP) - which demon of a group speaks.  BTLP only.
 *   0x8006C4D0 BtlPickTalkTarget
 *
 * A negotiation involves a set of enemies rather than one, so something has to
 * choose who the party is actually talking to. The mask is narrowed three
 * times over: anything carrying an ailment drops out, then everything but the
 * frontmost row, then everything but the leftmost of what is left. The lowest
 * slot still set is the answer, and -1 when the whole set has gone - which is
 * what BtlUpdateVoices watches for to know a line has finished travelling
 * down the group.
 *
 * Row and column come off the display object rather than the actor, so the
 * choice follows where the demons are actually standing.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* Wider than any column the grid uses, so the first one tested always wins. */
#define TALK_COL_NONE 8

extern BtlActor g_btl_enemies[];

short BtlPickTalkTarget(short mask)
{
    BtlActor *e;
    int       i;
    int       row;
    int       col;
    /* The narrowing loops compare against a copy of the best taken after the
       counter is reset, and the best itself is read back through a byte - both
       are the shape the original has, and folding either away costs the
       match. */
    int       keep;
    char      here;

    col = TALK_COL_NONE;
    row = 0;

    e = g_btl_enemies;
    i = 0;
    do {
        if (*(signed char *)&e->c.status != 0) {
            mask &= ~(1 << i);
        }
        i++;
        e++;
    } while (i < BTL_ENEMIES);

    e = g_btl_enemies;
    i = 0;
    do {
        if (((mask >> i) & 1) != 0) {
            if (row <= e->obj->row) {
                here = e->obj->row;
                row = here;
            }
        }
        i++;
        e++;
    } while (i < BTL_ENEMIES);

    e = g_btl_enemies;
    i = 0;
    keep = row;
    do {
        if (((mask >> i) & 1) != 0 && keep != e->obj->row) {
            mask &= ~(1 << i);
        }
        i++;
        e++;
    } while (i < BTL_ENEMIES);

    e = g_btl_enemies;
    i = 0;
    do {
        if (((mask >> i) & 1) != 0) {
            if (e->obj->col2 <= col) {
                here = e->obj->col2;
                col = here;
            }
        }
        i++;
        e++;
    } while (i < BTL_ENEMIES);

    e = g_btl_enemies;
    i = 0;
    keep = col;
    do {
        if (((mask >> i) & 1) != 0 && keep != e->obj->col2) {
            mask &= ~(1 << i);
        }
        i++;
        e++;
    } while (i < BTL_ENEMIES);

    i = 0;
    while (i < BTL_ENEMIES) {
        if (((mask >> i) & 1) != 0) {
            break;
        }
        i++;
    }
    if (i == BTL_ENEMIES) {
        return -1;
    }
    return i;
}
