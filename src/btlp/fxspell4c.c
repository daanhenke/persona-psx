/* Persona 1 (JP) - one move that borrows another's effect.  BTLP only.
 *   0x800BBFBC BtlFxStart4C
 *
 * A start handler out of g_btl_spell_fx that does nothing but call move 0x33's
 * handler and answer what it built. The table could have pointed both moves at
 * the one handler - it does exactly that elsewhere - so the call is the source's
 * shape rather than the table's, and it costs the move its own entry point.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart4C(void)
{
    return BtlFxStart33();
}
