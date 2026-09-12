/* Persona 1 (JP) - the move drawn as five records stacked on the fighter aimed
 * at.  BTLP only.
 *   0x800BBB88 BtlFxStart47
 *
 * A start handler out of g_btl_spell_fx. Five records are opened on the one
 * position - the fighter's own - each threaded onto the one before it and each
 * arriving four frames later than the last, so the stack builds up rather than
 * appearing at once. Every record but the last is drawn semi-transparent: the
 * upload slot the effect's artwork is drawn from is rewritten with the page
 * behind it and the blend bits set, and the record is marked so the drawing
 * side picks the transparent path.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* How much later each record arrives than the one below it, in frames. */
#define FX_47_STAGGER 4

/* How many records the stack is made of, counted down. */
#define FX_47_LAST 4

/* The upload slot the effect's artwork is drawn from, and the one whose page
   is copied into it. */
#define FX_47_SLOT 1
#define FX_47_FROM 29

/* The blend bits set on that page, and what the record carries so the drawing
   side reads them. */
#define FX_47_TRANS   0x60
#define FX_47_UNKCD   1

BtlObj *BtlFxStart47(void)
{
    BtlObj *o;
    BtlObj *prev;
    long    pos[3];
    int     i;

    pos[0] = g_btl_actors[g_btl_fx_target].obj->x;
    pos[1] = g_btl_actors[g_btl_fx_target].obj->y;
    pos[2] = 0;
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    i = FX_47_LAST;
    prev = NULL;
    do {
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, prev, FX_OBJ_DRAW, 0, pos,
                        FX_OBJ_CD, FX_OBJ_CE);
        o->attr = FX_OBJ_ATTR;
        o->mark_num = i;
        o->timer = i * FX_47_STAGGER;
        o->attached = prev;
        if (i != 0) {
            g_btl_tpage[FX_47_SLOT] = g_btl_tpage[FX_47_FROM] | FX_47_TRANS;
            o->unkCD = FX_47_UNKCD;
        }
        prev = o;
    } while (--i >= 0);
    return o;
}
