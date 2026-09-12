/* Persona 1 (JP) - closing the gap the dead leave at the back of the enemy
 * grid.  BTLP only.
 *   0x80095008 BtlPackEnemyGrid
 *
 * Nothing happens unless the back row is clear - and the test for that reads
 * only the first five of its nine cells, which is what the image does. With
 * the back row clear the rows are walked forward until one with anything on it
 * turns up, the grid is wiped, and every live enemy is moved back by however
 * many rows that was and written into the grid at its new cell.
 *
 * The move is a glide rather than a jump: the row step is turned into a
 * per-frame amount and the record is put on the motion that spends it over
 * sixteen frames.
 */
#include <decomp/include_asm.h>
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>

extern u_char g_btl_grid[];

/* Nine cells to a row and five rows, and the value an empty cell carries. The
   last cell is the one the wipe counts down from. */
#define GRID_WIDTH   9
#define GRID_ROWS    5
#define GRID_LAST    0x2C
#define GRID_BACK    ((GRID_ROWS - 1) * GRID_WIDTH)
#define GRID_EMPTY   0xFF

/* How much of the back row the first test reads. */
#define GRID_PEEK 5

/* The motion a moved enemy is put on, and how many frames it takes. */
#define PACK_MOTION 0xD
#define PACK_FRAMES 0x10

/* Both searches are written as a label and a goto rather than as loops. As
   do/whiles with a break gcc rotates each of them, peeling the first cell's
   load out above the loop and jumping into the test - thirteen instructions
   the image does not have, and 72.88% against 86.18% this way. What is still
   out is the grid's own address: the image lifts it into a saved register for
   the second search and gcc here folds the pointer back to the symbol and
   builds the address afresh at the one use. */
#ifdef NON_MATCHING
void BtlPackEnemyGrid(void)
{
    BtlObj *o;
    u_char *p;
    int     clear;
    int     found;
    int     cell;
    int     col;
    int     row;
    int     shift;
    int     fill;
    int     base;
    int     i;

    fill = GRID_EMPTY;
    clear = 1;
    cell = GRID_BACK;
peek:
    if (g_btl_grid[cell] != fill) {
        clear = 0;
        goto peeked;
    }
    cell++;
    if (cell < GRID_BACK + GRID_PEEK) {
        goto peek;
    }
peeked:
    if (clear == 0) {
        return;
    }

    found = 0;
    fill = GRID_EMPTY;
    row = GRID_ROWS - 1;
    cell = GRID_BACK;
scan:
    col = 0;
    base = cell;
cols:
    if (g_btl_grid[base + col] != fill) {
        found = 1;
        goto scanned;
    }
    col++;
    if (col < GRID_WIDTH) {
        goto cols;
    }
scanned:
    if (found == 0) {
        row--;
        cell -= GRID_WIDTH;
        if (row >= 0) {
            goto scan;
        }
    }

    fill = GRID_EMPTY;
    cell = GRID_LAST;
    p = &g_btl_grid[GRID_LAST];
    do {
        *p = fill;
        cell--;
        p--;
    } while (cell >= 0);

    shift = (GRID_ROWS - 1) - row;
    i = 0;
    do {
        if (g_btl_combatants[i].c.key != 0) {
            o = g_btl_combatants[i].obj;
            o->step_y = shift * PLACE_ROW_H * PLACE_FIXED / PACK_FRAMES;
            g_btl_combatants[i].obj->row += shift;
            g_btl_combatants[i].obj->motion = PACK_MOTION;
            g_btl_combatants[i].obj->phase = 0;
            g_btl_combatants[i].obj->steps = PACK_FRAMES;
            o = g_btl_combatants[i].obj;
            g_btl_grid[o->row * GRID_WIDTH + o->col2] =
                g_btl_combatants[i].c.key;
        }
        i++;
    } while (i < BTL_ENEMIES);
}
#else
INCLUDE_ASM("btlp/nonmatchings/packgrid", BtlPackEnemyGrid);
#endif
