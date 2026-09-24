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
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>

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

/* One flag serves both searches and one counter every walk but the columns,
   the way BtlAfterTalk is written for the party - the image keeps each in a
   single register throughout. The step is shifted into fixed point rather
   than multiplied by PLACE_FIXED: as a product gcc folds the division by
   sixteen into it, and the image divides at run time. */
void BtlPackEnemyGrid(void)
{
    BtlObj *o;
    u_char *p;
    int     flag;
    int     col;
    int     row;
    int     fill;
    int     i;

    i = GRID_BACK;
    flag = 1;
    for (; i < GRID_BACK + GRID_PEEK; i++) {
        if (g_btl_grid[i] != GRID_EMPTY) {
            flag = 0;
            break;
        }
    }
    if (flag == 0) {
        return;
    }

    flag = 0;
    row = GRID_ROWS - 1;
    do {
        for (col = 0; col < GRID_WIDTH; col++) {
            if (g_btl_grid[row * GRID_WIDTH + col] != GRID_EMPTY) {
                flag = 1;
                break;
            }
        }
        if (flag) {
            break;
        }
        row--;
    } while (row >= 0);

    fill = GRID_EMPTY;
    i = GRID_LAST;
    p = &g_btl_grid[GRID_LAST];
    do {
        *p = fill;
        i--;
        p--;
    } while (i >= 0);

    i = 0;
    do {
        if (g_btl_combatants[i].c.key != 0) {
            g_btl_combatants[i].obj->step_y =
                (((GRID_ROWS - 1) - row) * PLACE_ROW_H << 16) / PACK_FRAMES;
            g_btl_combatants[i].obj->row -= row - (GRID_ROWS - 1);
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
