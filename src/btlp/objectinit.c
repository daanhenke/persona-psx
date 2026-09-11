/* Persona 1 (JP) - bringing the battle display object pool up.  BTLP only.
 *   0x80080820 BtlInitObjects
 *
 * A unit of its own; the pool's list edits are in object.c and objectlist.c.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/sound.h>

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

/* What a fresh record starts at: all three scales at unity and the colour at
   half, which is what the drawers read as "leave it alone". */
#define BTL_ALLOC_SCALE 0x1000
#define BTL_ALLOC_RGB   0x80



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
    int     end;

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

    i = 0;
    obj = g_btl_obj_pool;
    for (; i < BTL_OBJ_GROUPS; i++) {
        obj->attr = BTL_OBJ_INUSE;
        obj->kind = BTL_OBJ_HEAD;
        obj->prev = 0;
        obj->next = 0;
        g_btl_obj_tail[i] = obj;
        obj++;
        n = 1;
        while (n < g_btl_obj_count[i]) {
            n++;
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
        end = BTL_FRAME_END;
    } while (frame < end);
    return 1;
}

/* Takes the first free record of a group and fills it in.
 *
 * The search starts at `after` when the caller has one - which is how a spawn
 * hangs a shadow immediately behind the thing it belongs to - and otherwise at
 * the record past the group's own head. It walks forward until it finds one
 * whose attribute word is clear, and gives up when it reaches the next
 * group's head rather than running into it.
 *
 * The template is an array and `index` picks the entry, so one table serves a
 * whole family. Everything else the record needs is either handed in or comes
 * off the template's first script step: the two signed bytes at the end of it
 * are the shift the object starts with, sixteen places up.
 */
BtlObj *BtlObjAlloc(const BtlObjDef *defs, int group, BtlObj *after, int draw,
                    int index, const long *pos, int unkCD, int unkCE)
{
    BtlObj *obj;
    BtlObj *tail;

    defs += index;
    if (after == 0) {
        after = &g_btl_obj_pool[g_btl_obj_first[group]] + 1;
    }
    obj = after;
    for (;;) {
        if ((obj->kind & BTL_OBJ_HEAD) != 0) {
            return 0;
        }
        if (obj->attr == 0) {
            tail = g_btl_obj_tail[group];
            tail->next = obj;
            obj->prev = tail;
            obj->next = 0;
            g_btl_obj_tail[group] = obj;

            /* A template that does not say it is static starts out
               animating. */
            if ((defs->attr & BTL_OBJ_STATIC) != 0) {
                obj->attr = defs->attr | BTL_OBJ_INUSE;
            } else {
                obj->attr = defs->attr | BTL_OBJ_INUSE | BTL_OBJ_ANIMATING;
            }
            obj->scripts = 0;
            obj->x = pos[0];
            obj->y = pos[1];
            obj->z = pos[2];
            obj->x2 = pos[0];
            obj->y2 = pos[1];
            obj->z2 = pos[2];
            obj->unk28 = 0;
            obj->unk2C = 0;
            obj->unk30 = 0;
            /* Read again for the second byte rather than kept: the store
               between them is what makes the original go back for it. */
            if (defs->scripts != 0) {
                obj->shift = ((const BtlSeqStep *)defs->scripts)->arg1 << 16;
                obj->shift_x = ((const BtlSeqStep *)defs->scripts)->arg0 << 16;
            } else {
                obj->shift_x = 0;
                obj->shift = 0;
            }
            obj->scale_to = 0;
            obj->attached = 0;
            obj->shadow = 0;
            obj->unk54 = 0;
            obj->unk58 = 0;
            obj->mark = 0;
            obj->unk60 = 0;
            obj->script = (BtlSeqStep *)defs->scripts;
            obj->last = ((const BtlSeqStep *)defs->scripts)->value;
            obj->unk70 = 0;
            obj->unk72 = 0;
            obj->unk74 = 0;
            obj->scale_x = BTL_ALLOC_SCALE;
            obj->scale_y = BTL_ALLOC_SCALE;
            obj->scale_z = BTL_ALLOC_SCALE;
            obj->col2 = 0;
            obj->row = 0;
            obj->kind = index;
            obj->children = 0;
            obj->step = 0;
            obj->unkB8 = 0;
            obj->age = 0;
            obj->unkBC = 0;
            obj->timer = 0;
            obj->rgb[0] = BTL_ALLOC_RGB;
            obj->rgb[1] = BTL_ALLOC_RGB;
            obj->rgb[2] = BTL_ALLOC_RGB;
            obj->rgb_to[0] = BTL_ALLOC_RGB;
            obj->rgb_to[1] = BTL_ALLOC_RGB;
            obj->rgb_to[2] = BTL_ALLOC_RGB;
            obj->fade = 0;
            obj->unkCD = unkCD;
            obj->unkCE = unkCE;
            obj->group = group;
            obj->draw = draw;
            obj->motion = 0;
            obj->mark_num = 0;
            obj->unkD3 = 0;
            obj->phase = 0;
            return obj;
        }
        obj++;
    }
}
