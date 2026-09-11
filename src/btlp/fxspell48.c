/* Persona 1 (JP) - four moves that all borrow 0x47's effect.  BTLP only.
 *   0x800BBCA8 BtlFxStart48  0x800BBCC8 BtlFxStart49
 *   0x800BBCE8 BtlFxStart4A  0x800BBD08 BtlFxStart4B
 *
 * Four start handlers out of g_btl_spell_fx, all four the same call. With 0x44
 * and 0x45 in front of them that is six moves reaching one handler through six
 * entry points of their own, where the table could have named it six times.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart48(void)
{
    return BtlFxStart47();
}

BtlObj *BtlFxStart49(void)
{
    return BtlFxStart47();
}

BtlObj *BtlFxStart4A(void)
{
    return BtlFxStart47();
}

BtlObj *BtlFxStart4B(void)
{
    return BtlFxStart47();
}
