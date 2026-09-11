/* Persona 1 (JP) - changing a display object.  BTLP only.
 *   0x800C46D0 BtlObjSetMarkNum  0x800C4704 BtlObjSetKind
 *   0x800C4738 BtlObjSetPos
 *
 * Three more of the shape objset.c through objset3.c are made of: write the
 * field, then hand the same value to whatever is attached.
 *
 * Two of them take a value narrower than the register it arrives in and the
 * recursive call passes it on masked, which is what the prototype's own type
 * is doing here rather than the assignment's.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>

void BtlObjSetMarkNum(BtlObj *obj, u_char num)
{
    obj->mark_num = num;
    if (obj->attached != 0) {
        BtlObjSetMarkNum(obj->attached, num);
    }
}

void BtlObjSetKind(BtlObj *obj, u_char kind)
{
    obj->kind = kind;
    if (obj->attached != 0) {
        BtlObjSetKind(obj->attached, kind);
    }
}

void BtlObjSetPos(BtlObj *obj, long x, long y, long z)
{
    obj->x = x;
    obj->y = y;
    obj->z = z;
    if (obj->attached != 0) {
        BtlObjSetPos(obj->attached, x, y, z);
    }
}
