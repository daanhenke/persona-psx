/* Persona 1 (JP) - the display object list.  BTLP only.
 *   0x80080D54 BtlObjFree     0x80080E7C BtlObjSetScript
 *   0x80080DAC BtlObjMoveBefore  0x80080ED0 BtlObjLast
 *   0x80080E0C BtlObjClone    0x80080F0C BtlObjChainAtMotion
 *
 * Everything here is a list edit rather than a change to what a record draws;
 * those are in objset.c. Bringing the pool up is in objectinit.c.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>

/* Takes a record out of its list. Returns whether there was one to take, which
   is what lets the callers pass a pointer they have not checked. */
int BtlObjFree(BtlObj *obj)
{
    int freed;

    freed = 0;
    if (obj != 0) {
        obj->attr = 0;
        obj->prev->next = obj->next;
        if (obj->next != 0) {
            obj->next->prev = obj->prev;
        } else {
            g_btl_obj_tail[obj->group] = obj->prev;
        }
        freed = 1;
    }
    return freed;
}

/* Relinks `obj` immediately before `at`, so it draws behind it. */
int BtlObjMoveBefore(BtlObj *at, BtlObj *obj)
{
    BtlObj *before;

    obj->prev->next = obj->next;
    if (obj->next != 0) {
        obj->next->prev = obj->prev;
    } else {
        g_btl_obj_tail[obj->group] = obj->prev;
    }
    before = at->prev;
    at->prev = obj;
    before->next = obj;
    obj->prev = before;
    obj->next = at;
    return 1;
}

/* A second record in the same group, linked in behind this one and running the
   same script from the same place. The template is built on the stack because
   the only thing it has to carry over is the script - the copy is meant to be
   moved and recoloured afterwards, which is what its callers do. */
BtlObj *BtlObjClone(BtlObj *obj)
{
    BtlObjDef def;
    BtlObj   *copy;

    def.attr = 0;
    def.scripts = (const u_long **)obj->script;
    copy = BtlObjAlloc(&def, obj->group, obj, obj->draw, 0, &obj->x,
                       obj->unkCD, obj->unkCE);
    copy->last = obj->last;
    return copy;
}

/* Points an object at an animation script and arms it. The script is walked
   once here to find its last step, whose first word is kept so the player can
   tell when it has finished without walking it again. A null script leaves the
   object alone, which is what lets callers pass one they have not checked. */
int BtlObjSetScript(BtlObj *obj, BtlSeqStep *script)
{
    if (script != 0) {
        obj->script = script;
        while ((script->flags & BTL_SEQ_MORE) != 0) {
            script++;
        }
        obj->last = script->value;
        obj->step = 0;
        obj->attr = (obj->attr | BTL_OBJ_ANIMATING) & ~BTL_OBJ_STATIC;
    }
    return 1;
}

/* The far end of a chain of attachments. Callers want both ends of it, so the
   one before last is left in g_btl_obj_prev rather than being worked out
   again. */
BtlObj *BtlObjLast(BtlObj *obj)
{
    g_btl_obj_prev = obj;
    while (obj->attached != 0) {
        g_btl_obj_prev = obj;
        obj = obj->attached;
    }
    return obj;
}

/* Whether the whole chain is on one motion. A chain that is not there counts
   as agreeing, which is what lets a caller ask about a marker it may never
   have spawned. */
int BtlObjChainAtMotion(BtlObj *obj, u_char motion)
{
    while (obj != 0) {
        if (obj->motion != motion) {
            return 0;
        }
        obj = obj->attached;
    }
    return 1;
}
