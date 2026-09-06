/* Persona 1 (JP) - the battle overlay's display objects.  BTLP only.
 *   0x80080820 BtlInitObjects   0x80080E7C BtlObjSetScript
 *   0x80080D54 BtlObjFree       0x80080DAC BtlObjMoveBefore
 *
 * Everything the battle draws - a character, a spell effect, a floating number
 * - is one 0xD8-byte record out of a single pool. The pool is cut into six
 * groups by g_btl_obj_count, and each group is a doubly linked list threaded
 * through the records themselves, with the last one in g_btl_obj_tail. Drawing
 * order is the list order, which is why moving a record is a list operation
 * rather than a sort key.
 *
 * The attribute word at +0 doubles as the in-use flag: BtlObjAlloc takes the
 * first record in the group whose attribute is clear, and BtlObjFree clears it
 * again.
 *
 * Each group opens with a head record that is never allocated - it carries
 * BTL_OBJ_HEAD and stays in use for the life of the overlay. Allocation starts
 * one past its group's head and stops when it reaches the next one, so the
 * heads are the group boundaries as well as the list anchors.
 */
#include <types.h>
#include <libgpu.h>

/* One step of an animation script. The high byte of `flags` is set on every
   step but the last, which is how the end is found. */
typedef struct {
    /* 0x0 */ u_long  value;
    /* 0x4 */ u_short flags;
    /* 0x6 */ u_short pad;
} BtlSeqStep;                          /* 8 bytes */

#define BTL_SEQ_MORE 0xFF00

typedef struct BtlObj {
    /* 0x00 */ u_long         attr;    /* zero when the record is free */
    /* 0x04 */ u_char         pad04[0x40];
    /* 0x44 */ struct BtlObj *prev;
    /* 0x48 */ struct BtlObj *next;
    /* 0x4C */ u_char         pad4C[0x18];
    /* 0x64 */ BtlSeqStep    *script; /* animation script                 */
    /* 0x68 */ u_long         last;    /* first word of the script's last step */
    /* 0x6C */ u_char         pad6C[0x46];
    /* 0xB2 */ u_short        kind;    /* BTL_OBJ_HEAD marks a list head   */
    /* 0xB4 */ u_char         padB4[2];
    /* 0xB6 */ short          step;    /* how far into the script it is    */
    /* 0xB8 */ u_char         padB8[0x17];
    /* 0xCF */ u_char         group;   /* which list the record belongs to */
    /* 0xD0 */ u_char         padD0[8];
} BtlObj;                              /* 0xD8 bytes */

#define BTL_OBJ_GROUPS 6
#define BTL_OBJ_INUSE  0x80000000   /* attribute bit that marks a record taken */

/* The kind field's top bit is not a kind at all: it marks the record that
   stands at the head of a group's list. BtlObjAlloc starts one record past it
   and stops at the next one, so a head doubles as the previous group's end. */
#define BTL_OBJ_HEAD   0x8000

/* Two more attribute bits, always changed together. BtlObjSetScript sets the
   first and clears the second when it arms a script, and BtlObjAlloc turns the
   first on for a record whose template has the second clear - so the pair says
   whether the object is running a script or standing still. */
#define BTL_OBJ_ANIMATING 0x10000000
#define BTL_OBJ_STATIC    0x20000000

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
