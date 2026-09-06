/* Persona 1 (JP) - the battle overlay's display object pool.  BTLP only.
 *   0x80080820 BtlInitObjects   0x80080E7C BtlObjSetScript
 *   0x80080D54 BtlObjFree       0x80080DAC BtlObjMoveBefore
 *   0x80080ED0 BtlObjLast
 *
 * The pool and the record are described in the header; this file is the pool's
 * own operations - bringing it up, arming a script, and the two list edits.
 */
#include <types.h>
#include <libgpu.h>
#include <persona/btlp/object.h>

/* Two frame buffers of primitives, laid out end to end from g_btl_prim_pool
   and pre-initialised so nothing has to issue SetSprt mid-frame. */
#define BTL_FRAME_STRIDE 0xE660
#define BTL_FRAME_END    0x1CCC0

#define BTL_SPRT_OFF     0x70
#define BTL_SPRT_N       0x1C2
#define BTL_TILE_OFF     0x2398
#define BTL_TILE_N       0x10
#define BTL_POLYFT4_OFF  0x2498
#define BTL_POLYFT4_N    0x1C2
#define BTL_POLYF4_OFF   0x6AE8
#define BTL_POLYF4_N     0x10
#define BTL_POLYG4_OFF   0x6C68
#define BTL_POLYG4_N     0x40
#define BTL_LINEG2_OFF   0x7568
#define BTL_LINEG2_N     0x10

#define BTL_SLOTS 32

extern BtlObj  *g_btl_obj_tail[];
extern BtlObj  *g_btl_obj_pool;
extern BtlObj  *g_btl_obj_prev;
extern u_short  g_btl_obj_count[];
extern u_char  *g_btl_prim_pool;
extern short    g_btl_slot_owner[];

extern SPRT     *g_btl_sprt_next;
extern TILE     *g_btl_tile_next;
extern POLY_FT4 *g_btl_polyft4_next;
extern POLY_F4  *g_btl_polyf4_next;
extern POLY_G4  *g_btl_polyg4_next;
extern LINE_G2  *g_btl_lineg2_next;

/* Overlay entry. Every slot goes back to unowned, each group's list is reduced
   to its first record, and both frame buffers' primitives are re-tagged. */
int BtlInitObjects(void)
{
    BtlObj *obj;
    short  *slot;
    int     i;
    int     n;
    int     frame;
    short   none;

    /* One counter runs the slot fill and then the group loop; it is the same
       register in the original, so it is the same variable. The fill value is
       a local for the same reason - it is materialised before either. */
    none = -1;
    i = BTL_SLOTS - 1;
    slot = &g_btl_slot_owner[BTL_SLOTS - 1];
    do {
        *slot = none;
        i--;
        slot--;
    } while (i >= 0);

    obj = g_btl_obj_pool;
    for (i = 0; i < BTL_OBJ_GROUPS; i++) {
        obj->attr = BTL_OBJ_INUSE;
        obj->kind = BTL_OBJ_HEAD;
        obj->prev = 0;
        obj->next = 0;
        g_btl_obj_tail[i] = obj;
        obj++;
        for (n = 1; n < g_btl_obj_count[i]; n++) {
            obj->attr = 0;
            obj->kind = 0;
            obj++;
        }
    }
    obj->attr = BTL_OBJ_INUSE;
    obj->kind = BTL_OBJ_HEAD;

    frame = 0;
    do {
        g_btl_sprt_next = (SPRT *)(g_btl_prim_pool + frame + BTL_SPRT_OFF);
        for (n = 0; n < BTL_SPRT_N; n++) {
            SetSprt(g_btl_sprt_next);
            SetShadeTex(g_btl_sprt_next, 0);
            g_btl_sprt_next++;
        }
        g_btl_tile_next = (TILE *)(g_btl_prim_pool + frame + BTL_TILE_OFF);
        for (n = 0; n < BTL_TILE_N; n++) {
            SetTile(g_btl_tile_next);
            g_btl_tile_next++;
        }
        g_btl_polyft4_next =
            (POLY_FT4 *)(g_btl_prim_pool + frame + BTL_POLYFT4_OFF);
        for (n = 0; n < BTL_POLYFT4_N; n++) {
            SetPolyFT4(g_btl_polyft4_next);
            SetShadeTex(g_btl_polyft4_next, 0);
            g_btl_polyft4_next++;
        }
        g_btl_polyf4_next =
            (POLY_F4 *)(g_btl_prim_pool + frame + BTL_POLYF4_OFF);
        for (n = 0; n < BTL_POLYF4_N; n++) {
            SetPolyF4(g_btl_polyf4_next);
            g_btl_polyf4_next++;
        }
        g_btl_polyg4_next =
            (POLY_G4 *)(g_btl_prim_pool + frame + BTL_POLYG4_OFF);
        for (n = 0; n < BTL_POLYG4_N; n++) {
            SetPolyG4(g_btl_polyg4_next);
            g_btl_polyg4_next++;
        }
        g_btl_lineg2_next =
            (LINE_G2 *)(g_btl_prim_pool + frame + BTL_LINEG2_OFF);
        for (n = 0; n < BTL_LINEG2_N; n++) {
            SetLineG2(g_btl_lineg2_next);
            g_btl_lineg2_next++;
        }
        frame += BTL_FRAME_STRIDE;
    } while (frame < BTL_FRAME_END);
    return 1;
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
