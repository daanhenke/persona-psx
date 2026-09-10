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


extern SPRT     *g_btl_sprt_next;
extern TILE     *g_btl_tile_next;
extern POLY_F4  *g_btl_polyf4_next;
extern POLY_G4  *g_btl_polyg4_next;
extern LINE_G2  *g_btl_lineg2_next;

/* Overlay entry. Every slot goes back to unowned, each group's list is reduced
   to its first record, and both frame buffers' primitives are re-tagged. */
#ifdef NON_MATCHING
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
#else
INCLUDE_ASM("btlp/nonmatchings/objectinit", BtlInitObjects);
#endif
