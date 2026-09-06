/* Persona 1 (JP) - putting a party member on the grid.  BTLP only.
 *   0x800A4760 BtlPlaceMember
 *
 * The battlefield is a grid, and this turns a cell into a position outright -
 * no glide, so both copies of x and y get the same value. Thirty pixels to a
 * column, twenty to a row, and the party's origin is a column left and a row
 * down of the world's; the enemies are laid out the same way from their own
 * origin further back.
 *
 * The member's shadow is a display object of its own hanging off the record,
 * and it is moved with the member rather than following behind, so the two are
 * never a frame apart.
 */
#include <types.h>
#include <persona/btlp/actor.h>

/* Thirty pixels to a column from -0x3C, twenty to a row from +0x3C. */
#define PLACE_COL_W   0x1E
#define PLACE_COL_ORG (-0x3C)
#define PLACE_ROW_H   0x14
#define PLACE_ROW_ORG 0x3C

/* Positions are 16.16. The column is stored on the object doubled, because the
   grid is kept to half-column resolution. */
#define PLACE_FIXED 0x10000

void BtlPlaceMember(int slot, short col, short row)
{
    BtlObj *o;
    int     x;
    int     y;

    x = (col * PLACE_COL_W + PLACE_COL_ORG) * PLACE_FIXED;
    y = (row * PLACE_ROW_H + PLACE_ROW_ORG) * PLACE_FIXED;
    o = g_btl_actors[slot].obj;
    o->x = x;
    o->x2 = x;
    o->y = y;
    o->y2 = y;
    o->col2 = col * 2;
    o->row = row;
    o = g_btl_actors[slot].obj->shadow;
    o->x = x;
    o->x2 = x;
    o->y = y;
    o->y2 = y;
    o->col2 = col * 2;
    o->row = row;
}
