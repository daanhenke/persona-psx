/* Persona 1 (JP) - the last entry of the ailment turn table.  BTLP only.
 *   0x80096180 BtlAilmentTurnWolf
 *
 * Entry 23, and the only one behind the madness handler. It does nothing at
 * all: a fighter under it takes its turn as it stands. It is here because the
 * table is indexed straight by the ailment code and so needs an entry for
 * every one of the twenty-four.
 *
 * It is eight bytes on the tail of BtlAilmentTurnMad, which is why the
 * boundary finder gave them to that routine and the table's last entry read as
 * a bare address.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/status.h>

void BtlAilmentTurnWolf(BtlActor *a, u_char *act)
{
}
