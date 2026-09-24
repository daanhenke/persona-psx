/* Persona 1 (JP) - the placement menu's cursors.  BTLP only.
 *   0x800A4B38 BtlStandPreset  0x800A4D0C BtlPlacePreset
 *   0x800A4E00 BtlMarkMovedMembers
 *
 * Three helpers of BtlPlaceMenu.
 *
 * BtlStandPreset stands the party where one layout puts them: the whole party
 * is hidden, and every member the layout places who is still in the fight is
 * shown again with its shadow and moved onto its cell; a member who is down
 * stays hidden. Only the live grid gives a member its shadow back - a stored
 * layout being previewed draws the fighters without one. Once everybody is
 * placed the ailment marks, the pick cursors and the markers are brought back
 * into line with where they now stand.
 *
 * BtlPlacePreset shows one of the eight stored layouts on the menu's own
 * copy of the grid: every pick cursor is hidden first, and then each member
 * the layout puts on a cell has its cursor moved there and shown again. The
 * grid is drawn sixteen pixels to a column from PRESET_X and eight to a row
 * from PRESET_Y.
 *
 * BtlMarkMovedMembers compares the live grid with g_btl_formation_before, the
 * one the menu opened on, and puts BTL_MARKER_MOVED on every member still in the
 * fight who is standing somewhere else now - unless that member's marker is
 * already up - answering whether anybody moved at all.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/status.h>

/* Where the menu draws the grid, in pixels; positions are 16.16. */
#define PRESET_X      0xE8
#define PRESET_Y      0x78
#define PRESET_XPITCH 16
#define PRESET_YPITCH 8

extern BtlObj *g_btl_pick_cursors[];

/* The layout is reached as (g_btl_formation_preset + preset)->cell[k], which
   the image strength-reduces into a pointer stepped along the row; indexing
   the table, or the cells as one run, hoists the whole product instead. */
void BtlStandPreset(int preset)
{
    BtlObj *o;
    u_char  cell;
    int     row;
    int     col;
    int     k;

    BtlPartySetAttr(BTL_OBJ_HIDDEN);
    row = 0;
    k = 0;
    do {
        col = 0;
        do {
            cell = (g_btl_formation_preset + preset)->cell[k];
            if (cell != CELL_EMPTY) {
                o = g_btl_actors[cell].obj;
                if (g_btl_actors[cell].c.key != 0) {
                    if ((signed char)g_btl_actors[cell].c.status
                        != BTL_STATUS_DOWN) {
                        o->attr &= ~BTL_OBJ_HIDDEN;
                        o->shadow->attr &= ~BTL_OBJ_HIDDEN;
                        if (preset == PRESET_LIVE) {
                            o->attr &= ~BTL_OBJ_NO_SHADOW;
                            o->shadow->attr &= ~BTL_OBJ_HIDDEN;
                        } else {
                            o->attr |= BTL_OBJ_NO_SHADOW;
                            o->shadow->attr |= BTL_OBJ_HIDDEN;
                        }
                        BtlPlaceMember(cell, col, row);
                    } else {
                        o->attr |= BTL_OBJ_HIDDEN;
                    }
                }
            }
            col++;
            k++;
        } while (col < GRID_W);
        row++;
    } while (row < GRID_H);
    BtlShowAilmentMarks(1);
    BtlRefreshPickCursors();
    BtlBuildMarkers();
}

/* One counter for both walks: i hides the five cursors and then runs over
   the preset's cells, which is what shares its register with the column
   count the way the image has it. The layout is reached as
   (g_btl_formation_preset + preset)->cell[i], BtlStandPreset's spelling. */
void BtlPlacePreset(int preset)
{
    int  cell;
    long x;
    long y;
    int  row;
    int  col;
    int  i;

    i = 0;
    do {
        g_btl_pick_cursors[i++]->attr |= BTL_OBJ_HIDDEN;
    } while (i < PARTY_MAX);

    row = 0;
    i = 0;
    do {
        y = (row * PRESET_YPITCH + PRESET_Y) << 16;
        col = 0;
        do {
            x = (col * PRESET_XPITCH + PRESET_X) << 16;
            cell = (g_btl_formation_preset + preset)->cell[i];
            if (cell != CELL_EMPTY) {
                g_btl_pick_cursors[cell]->x = x;
                g_btl_pick_cursors[cell]->y = y;
                g_btl_pick_cursors[cell]->attr &= ~BTL_OBJ_HIDDEN;
            }
            col++;
            i++;
        } while (col < GRID_W);
        row++;
    } while (row < GRID_H);
}

int BtlMarkMovedMembers(void)
{
    BtlObj *o;
    int     cell;
    int     moved;
    int     i;

    i = 0;
    moved = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            o = g_btl_actors[i].obj;
            cell = o->row * GRID_W + (o->col2 >> 1);
            if (g_btl_formation[cell] != g_btl_formation_before[cell]
                && g_btl_actors[i].marker != BTL_MARKER_UP) {
                g_btl_actors[i].marker = BTL_MARKER_MOVED;
                moved = 1;
            }
        }
        i++;
    } while (i < BTL_PARTY);
    return moved;
}
