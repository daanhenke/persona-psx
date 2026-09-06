/* Persona 1 (JP) - putting the six-slot picker on screen.  BTLP only.
 *   0x800A98D0 BtlPickSpawn
 *
 * Twelve objects in two passes of six. The first pass gives every slot its own
 * template, so those six differ from each other; the second takes one template
 * for all six and hangs the matching first-pass object off its attached link.
 * g_btl_pick_objs keeps the second of each pair, which is why BtlPickHighlight
 * colours the shared piece directly and the per-slot piece through `attached`.
 *
 * Each call is handed the object the last one returned, so the twelve come out
 * of the pool in order and stay in that order in the group's list - which is
 * the order they are drawn in.
 */
#include <types.h>
#include <persona/btlp/object.h>

#define BTL_PICK_SLOTS 6

/* Which of the six object groups the picker lives in, and the one template
   the second pass takes for all six. */
#define BTL_PICK_GROUP 1
#define BTL_PICK_FRAME 6

/* Turned on for the per-slot piece, and for the one that carries it. */
#define BTL_PICK_ITEM_BIT  0x400
#define BTL_PICK_FRAME_BIT 0x40000080

/* Unity, in the two units BtlObjSetScale takes. */
#define BTL_PICK_SCALE_XY 0x100
#define BTL_PICK_SCALE_Z  0x1000

extern const BtlObjDef g_btl_pick_defs[];
extern const BtlObjDef g_btl_obj_defs[];
extern long            g_btl_pick_pos[][4];
extern BtlObj         *g_btl_pick_objs[];

/* The last two arguments end up as the bytes at +0xCD and +0xCE of the record;
   what they mean there is not settled, only where they go. */
extern BtlObj *BtlObjAlloc(const BtlObjDef *defs, int group, BtlObj *after,
                           int a3, int index, const long *pos, int p7, int p8);
extern void BtlObjSetScale(BtlObj *obj, long x, long y, long z);
extern void BtlObjSetAttr(BtlObj *obj, u_long bits);

void BtlPickSpawn(void)
{
    BtlObj *obj;
    int     i;

    i = 0;
    obj = 0;
    do {
        obj = BtlObjAlloc(g_btl_pick_defs, BTL_PICK_GROUP, obj, 1,
                          i, g_btl_pick_pos[i], 0x1F, 0x27);
        obj->attr |= BTL_PICK_ITEM_BIT;
        g_btl_pick_objs[i] = obj;
        i++;
    } while (i < BTL_PICK_SLOTS);

    i = 0;
    do {
        obj = BtlObjAlloc(g_btl_obj_defs, BTL_PICK_GROUP, obj, 1,
                          BTL_PICK_FRAME, g_btl_pick_pos[i], 0x19, 0x1E);
        obj->attached = g_btl_pick_objs[i];
        g_btl_pick_objs[i] = obj;
        BtlObjSetScale(obj, BTL_PICK_SCALE_XY, BTL_PICK_SCALE_XY,
                       BTL_PICK_SCALE_Z);
        BtlObjSetAttr(obj, BTL_PICK_FRAME_BIT);
        i++;
    } while (i < BTL_PICK_SLOTS);
}
