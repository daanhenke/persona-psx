/* Persona 1 (JP) - the grid a target is picked on.  BTLP only.
 *   0x800AA6E8 BtlSpawnPickGrid  0x800AA958 BtlDespawnPickGrid
 *   0x800AA9A0 BtlRefreshPickCursors
 *
 * Three things go up together. Two records stand behind everything - an
 * anchor that is never drawn and the backing plate - and both are put
 * straight out of the drawing pass; then five cursors, one per party member,
 * built back to front so that the first member's ends up at the head of the
 * chain; then the grid itself, eighteen cells in six rows of three.
 *
 * The cells all come from the same template, g_btl_obj_defs[8], with its
 * script table rewritten before each allocation - eighteen tables laid out
 * one after another at g_btl_grid_scripts - so one entry of the table draws
 * every cell and each cell still runs its own animation. Each cell also keeps
 * the address of g_btl_grid_tail in unk58, which is how the frame tick finds
 * the end of the chain.
 *
 * The grid is built once: g_btl_grid_tail holding a record is what says it is
 * already there, and a second call only puts the backing up again and
 * refreshes the cursors.
 *
 * BtlRefreshPickCursors is the per-frame half. A member who is not there, is
 * down, is out, or whose own object has been taken out of the drawing pass
 * has no cursor shown; anyone else gets theirs put on their object's grid
 * square, which is read back out of the object rather than kept.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* Party members, and the grid's shape. */
#define GRID_CURSORS 5
#define GRID_ROWS    6
#define GRID_COLS    3

/* Which of the six object groups all of this lives in. */
#define GRID_GROUP 1

/* Where the backing stands, and where the grid starts. Both axes step by a
   cell as the two loops walk. */
#define GRID_BACK_X  0x1080000
#define GRID_BACK_Y  0x880000
#define GRID_FIRST_X 0xE00000
#define GRID_FIRST_Y 0x780000
#define GRID_STEP    0x100000

/* The template index the cells take, and the pair of bytes every record of
   the grid is given. */
#define GRID_CELL_DEF 8
#define GRID_UNK_CD   0x19
#define GRID_UNK_CE   0x1E

/* The anchor is drawn by handler 10 and marked with a kind of its own so
   nothing mistakes it for a cell. */
#define GRID_ANCHOR_DRAW 10
#define GRID_ANCHOR_KIND 0xFF

/* A cursor is drawn at half size and carries the bit every piece of a built
   assembly does. */
#define GRID_CURSOR_SCALE 0x80
#define GRID_PIECE_BIT    0x400

/* The two angles a cell is given - three quarters of a turn and half of one -
   and the motion the whole grid runs. */
#define GRID_CELL_ROT_Y 0xC00
#define GRID_CELL_ROT_Z 0x800
#define GRID_MOTION     8

/* What the backing plate and the cursors are put away with. */
#define GRID_BACK_MOTION 1
#define GRID_STOP_MOTION 0xB

/* Where a cursor sits for a given grid square. */
#define GRID_CURSOR_X(col2) ((((col2) >> 1) * 0x10) + 0xE8)
#define GRID_CURSOR_Y(row)  (((row) * 8) + 0x78)

extern const BtlObjDef g_btl_shadow_defs[];
extern BtlObjDef       g_btl_obj_defs[];

/* Eighteen script tables, one per cell, two pointers each. */
extern const u_long *g_btl_grid_scripts[][2];

extern BtlObj *g_btl_grid_anchor;
extern BtlObj *g_btl_grid_back;
extern BtlObj *g_btl_grid_tail;
extern BtlObj *g_btl_pick_cursors[];

void BtlRefreshPickCursors(void);

void BtlSpawnPickGrid(void)
{
    long    pos[3];
    BtlObj *obj;
    BtlObj *prev;
    const u_long **script;
    long    y;
    int     i;
    int     row;
    int     col;

    pos[0] = GRID_BACK_X;
    pos[1] = GRID_BACK_Y;
    pos[2] = 0;
    obj = BtlObjAlloc(g_btl_shadow_defs, GRID_GROUP, 0, GRID_ANCHOR_DRAW,
                      GRID_CELL_DEF, pos, 0, 0);
    obj->kind         = GRID_ANCHOR_KIND;
    g_btl_grid_anchor = obj;
    obj->attr |= BTL_OBJ_HIDDEN;

    obj = BtlObjAlloc(g_btl_shadow_defs, GRID_GROUP, obj, 1, 2, pos,
                      GRID_UNK_CD, GRID_UNK_CE);
    obj->motion     = GRID_BACK_MOTION;
    g_btl_grid_back = obj;
    obj->attr |= BTL_OBJ_HIDDEN;

    if (g_btl_grid_tail == 0) {
        prev = 0;
        i    = GRID_CURSORS - 1;
        do {
            obj = BtlObjAlloc(g_btl_shadow_defs, GRID_GROUP, prev, 1, i + 3, pos,
                              GRID_UNK_CD, GRID_UNK_CE);
            obj->attached = prev;
            prev          = obj;
            obj->scale_x  = GRID_CURSOR_SCALE;
            obj->scale_y  = GRID_CURSOR_SCALE;
            obj->attr |= GRID_PIECE_BIT;
            g_btl_pick_cursors[i] = obj;
            i--;
        } while (i >= 0);

        script = g_btl_grid_scripts[0];
        prev   = 0;
        row    = 0;
        do {
            col = 0;
            y   = GRID_FIRST_Y;
            do {
                g_btl_obj_defs[GRID_CELL_DEF].scripts = script;
                pos[0] = GRID_FIRST_X + (row * GRID_STEP);
                pos[1] = y;
                obj = BtlObjAlloc(g_btl_obj_defs, GRID_GROUP, prev, 1,
                                  GRID_CELL_DEF, pos, GRID_UNK_CD, GRID_UNK_CE);
                obj->attached = prev;
                prev          = obj;
                y += GRID_STEP;
                col++;
                script += 2;
                obj->unk58 = (long)&g_btl_grid_tail;
                obj->rot.vy = GRID_CELL_ROT_Y;
                obj->rot.vz = GRID_CELL_ROT_Z;
                obj->motion = GRID_MOTION;
            } while (col < GRID_COLS);
            row++;
        } while (row < GRID_ROWS);
        g_btl_grid_tail = obj;
    }
    BtlObjSetMotion(g_btl_grid_tail, GRID_MOTION);
    BtlRefreshPickCursors();
}

/* The grid itself is left standing - only the backing and the anchor are
   handed back, and the first cursor is told to stop. */
void BtlDespawnPickGrid(void)
{
    BtlObjSetMotion(g_btl_pick_cursors[0], GRID_STOP_MOTION);
    BtlObjFree(g_btl_grid_back);
    BtlObjFree(g_btl_grid_anchor);
}

void BtlRefreshPickCursors(void)
{
    int i;

    i = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && (g_btl_actors[i].obj->attr & BTL_OBJ_HIDDEN) == 0) {
            g_btl_pick_cursors[i]->x =
                GRID_CURSOR_X(g_btl_actors[i].obj->col2) << 16;
            g_btl_pick_cursors[i]->y =
                GRID_CURSOR_Y(g_btl_actors[i].obj->row) << 16;
            g_btl_pick_cursors[i]->attr &= ~BTL_OBJ_HIDDEN;
        } else {
            g_btl_pick_cursors[i]->attr |= BTL_OBJ_HIDDEN;
        }
        i++;
    } while (i < GRID_CURSORS);
}
