/* Persona 1 (JP) - where the party's pick cursors stand.  BTLP only.
 *   0x800AAAB4 BtlPlacePickCursors
 *
 * The same pass BtlRefreshPickCursors makes, with one more answer. A member
 * who is there, is not down and is not out gets their cursor put on their
 * object's grid square at full brightness. A member who fails that is looked
 * at again: one who is there, is down, and carries unkDC still gets a cursor,
 * on the same square but at half brightness - that is the fallen member a
 * revival can still be aimed at. Anyone else has no cursor shown.
 *
 * The two live answers are the same seven writes with a different brightness,
 * and both read the object back through the record rather than keeping it.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

#define BTL_PICK_CURSORS 5

/* How bright a cursor is drawn, and how fast it gets there. */
#define PICK_CURSOR_LIVE 0x80
#define PICK_CURSOR_DOWN 0x40
#define PICK_CURSOR_FADE 0xFF

/* Where a cursor sits for a given grid square. */
#define PICK_CURSOR_X(col2) ((((col2) >> 1) * 0x10) + 0xE8)
#define PICK_CURSOR_Y(row)  (((row) * 8) + 0x78)

extern BtlObj *g_btl_pick_cursors[];

void BtlPlacePickCursors(void)
{
    BtlActor *a;
    int       i;

    i      = 0;
    a      = g_btl_actors;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            g_btl_pick_cursors[i]->x = PICK_CURSOR_X(a->obj->col2) << 16;
            g_btl_pick_cursors[i]->y = PICK_CURSOR_Y(a->obj->row) << 16;
            g_btl_pick_cursors[i]->attr &= ~BTL_OBJ_HIDDEN;
            g_btl_pick_cursors[i]->fade      = PICK_CURSOR_FADE;
            g_btl_pick_cursors[i]->rgb_to[0] = PICK_CURSOR_LIVE;
            g_btl_pick_cursors[i]->rgb_to[1] = PICK_CURSOR_LIVE;
            g_btl_pick_cursors[i]->rgb_to[2] = PICK_CURSOR_LIVE;
        } else if (a->c.key != 0
                   && (signed char)a->c.status == BTL_STATUS_DOWN
                   && (signed char)a->unkDC != 0) {
            g_btl_pick_cursors[i]->x = PICK_CURSOR_X(a->obj->col2) << 16;
            g_btl_pick_cursors[i]->y = PICK_CURSOR_Y(a->obj->row) << 16;
            g_btl_pick_cursors[i]->attr &= ~BTL_OBJ_HIDDEN;
            g_btl_pick_cursors[i]->fade      = PICK_CURSOR_FADE;
            g_btl_pick_cursors[i]->rgb_to[0] = PICK_CURSOR_DOWN;
            g_btl_pick_cursors[i]->rgb_to[1] = PICK_CURSOR_DOWN;
            g_btl_pick_cursors[i]->rgb_to[2] = PICK_CURSOR_DOWN;
        } else {
            g_btl_pick_cursors[i]->attr |= BTL_OBJ_HIDDEN;
        }
        i++;
        a++;
    } while (i < BTL_PICK_CURSORS);
}
