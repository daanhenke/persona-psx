/* Persona 1 (JP) - the two motions that scroll a list board by a page.
 * BTLP only.
 *   0x800AD4D8 BtlObjMotion06  0x800AD70C BtlObjMotion07
 *
 * Entries 6 and 7 of g_btl_obj_motion. The record is the frame of the list the
 * spell and item boards share, g_btl_list_open says which of the two it is
 * showing, and its step's rows are the text of both columns: rows 0 and 1 are
 * the column's first two lines and rows 10 and 11 the two under the page, and
 * rows 12, 13, 22 and 23 the same four in the second column.
 *
 * 06 scrolls forward. The page is built, and for three frames the first two
 * lines of each column are squeezed and slid down three pixels while the lines
 * under the page open by the same amount and the frame rises. Then the lines
 * are put back square and nine pixels up, the list moves on - two spells, or
 * to the next usable item - and is built again, and the frame drops back.
 *
 * 07 scrolls back: the list moves back and is built first, the lines are set
 * up squeezed shut with the frame twelve pixels up, and for four frames they
 * open three pixels a frame while the frame comes down.
 *
 * The rows are indexed by the line, rows[i + LIST_UNDER] and so on, not walked
 * by a pointer: gcc then reduces them all to one pointer based on the first
 * row, where a walked pointer is based on the last row the loop touches.
 */
#include <decomp/types.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>

/* The lines scrolled in each column, the row under the page, the second
   column's first row and the row under its page. */
#define LIST_LINES  2
#define LIST_UNDER  10
#define LIST_COLUMN 12
#define LIST_TAIL   22

/* A line's height, how far a frame of the scroll moves, how many frames each
   way takes, and how far 06 settles the lines. */
#define LIST_LINE_H     12
#define LIST_STEP       3
#define LIST_OUT_FRAMES 3
#define LIST_IN_FRAMES  4
#define LIST_SETTLE     9

void BtlObjMotion06(BtlObj *obj)
{
    BtlGfxText *rows;
    int         i;

    rows = ((BtlGfxTextList *)obj->last)->rows;
    switch (obj->phase) {
    case 0:
        if (g_btl_list_open != 0) {
            BtlBuildSpellLines(g_btl_spell_slot);
        } else {
            BtlBuildItemLines(g_btl_item_at);
        }
        obj->steps = 0;
        obj->phase++;
        /* fall through */
    case 1:
        if (obj->steps < LIST_OUT_FRAMES) {
            obj->y -= LIST_STEP << 16;
            for (i = 0; i < LIST_LINES; i++) {
                rows[i].h -= LIST_STEP;
                rows[i].v += LIST_STEP;
                rows[i].y += LIST_STEP;
                rows[i + LIST_UNDER].h += LIST_STEP;
                rows[i + LIST_COLUMN].h -= LIST_STEP;
                rows[i + LIST_COLUMN].v += LIST_STEP;
                rows[i + LIST_COLUMN].y += LIST_STEP;
                rows[i + LIST_TAIL].h += LIST_STEP;
            }
            obj->steps++;
            break;
        }
        obj->steps = 0;
        obj->phase++;
        /* fall through */
    case 2:
        for (i = 0; i < LIST_LINES; i++) {
            rows[i].h = LIST_LINE_H;
            rows[i].v = 0;
            rows[i + LIST_UNDER].h = 0;
            rows[i + LIST_COLUMN].h = LIST_LINE_H;
            rows[i + LIST_COLUMN].v = 0;
            rows[i + LIST_TAIL].h = 0;
            rows[i].y -= LIST_SETTLE;
            rows[i + LIST_COLUMN].y -= LIST_SETTLE;
        }
        if (g_btl_list_open != 0) {
            g_btl_spell_slot += LIST_LINES;
            BtlBuildSpellLines(g_btl_spell_slot);
        } else {
            g_btl_item_at = BtlNextUsableItem(g_btl_item_at);
            BtlBuildItemLines(g_btl_item_at);
        }
        obj->y += LIST_SETTLE << 16;
        obj->motion = 0;
        obj->phase = 0;
        break;
    }
}

void BtlObjMotion07(BtlObj *obj)
{
    BtlGfxText *rows;
    int         i;

    rows = ((BtlGfxTextList *)obj->last)->rows;
    switch (obj->phase) {
    case 0:
        if (g_btl_list_open != 0) {
            g_btl_spell_slot -= LIST_LINES;
            BtlBuildSpellLines(g_btl_spell_slot);
        } else {
            g_btl_item_at = BtlPrevUsableItem(g_btl_item_at);
            BtlBuildItemLines(g_btl_item_at);
        }
        obj->y -= LIST_LINE_H << 16;
        for (i = 0; i < LIST_LINES; i++) {
            rows[i].h = 0;
            rows[i + LIST_UNDER].h = LIST_LINE_H;
            rows[i + LIST_COLUMN].h = 0;
            rows[i + LIST_TAIL].h = LIST_LINE_H;
            rows[i].v += LIST_LINE_H;
            rows[i].y += LIST_LINE_H;
            rows[i + LIST_COLUMN].v += LIST_LINE_H;
            rows[i + LIST_COLUMN].y += LIST_LINE_H;
        }
        obj->steps = 0;
        obj->phase++;
        /* fall through */
    case 1:
        obj->y += LIST_STEP << 16;
        for (i = 0; i < LIST_LINES; i++) {
            /* The font row is read signed on the way down: as a plain byte
               gcc folds the step to +0xFD, where the image adds -3. */
            rows[i].h += LIST_STEP;
            rows[i].v = (signed char)rows[i].v - LIST_STEP;
            rows[i].y -= LIST_STEP;
            rows[i + LIST_UNDER].h -= LIST_STEP;
            rows[i + LIST_COLUMN].h += LIST_STEP;
            rows[i + LIST_COLUMN].v =
                (signed char)rows[i + LIST_COLUMN].v - LIST_STEP;
            rows[i + LIST_COLUMN].y -= LIST_STEP;
            rows[i + LIST_TAIL].h -= LIST_STEP;
        }
        if (++obj->steps >= LIST_IN_FRAMES) {
            obj->motion = 0;
            obj->phase = 0;
        }
        break;
    }
}
