/* Persona 1 (JP) - three moves that borrow the three handlers before them.
 * BTLP only.
 *   0x800BC33C BtlFxStart4D  0x800BC35C BtlFxStart4E  0x800BC37C BtlFxStart4F
 *
 * Three start handlers out of g_btl_spell_fx, each calling one of moves 0x31,
 * 0x32 and 0x33 in turn - the same three effects at another strength, which is
 * why the three borrowers were written together.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart4D(void)
{
    return BtlFxStart31();
}

BtlObj *BtlFxStart4E(void)
{
    return BtlFxStart32();
}

BtlObj *BtlFxStart4F(void)
{
    return BtlFxStart33();
}
