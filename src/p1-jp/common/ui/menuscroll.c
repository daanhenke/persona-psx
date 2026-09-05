/* Persona 1 (JP) - the cursor step for a list that scrolls three at a time.
 *
 *   DNG 0x80077714   ADV 0x80067FBC   S2D 0x8006773C
 *
 * The list holds one screenful; running the cursor past either end leaves it
 * where it was and moves the window instead, one row of three entries.
 */
#include <types.h>
#include <persona/common/menulist.h>

short MenuScrollCursor(MenuList *m, short *row, short first, short last,
                       u_short *offset)
{
    short old;
    short lo;
    short hi;
    short moved;

    old = m->cur;
    lo = m->lo;
    hi = m->hi;
    moved = MenuStepCursor(m);
    if (old == lo || old == hi) {
        if (moved) {
            if (m->cur == lo) {
                if (*row != first) {
                    *row = *row - 1;
                    *offset = *offset - 3;
                }
            } else if (m->cur == hi) {
                if (*row != last) {
                    *row = *row + 1;
                    *offset = *offset + 3;
                }
            }
        }
    }
    return moved;
}
