/* Persona 1 (JP) - where a fighter's whitening starts in its palette.
 * BTLP only.
 *   0x80097698 BtlActorClutFirst
 *
 * One byte per Char key out of g_btl_key_clut_first: nought for most, and one
 * for the four keys whose first palette entry has to be left alone.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/status.h>

int BtlActorClutFirst(const BtlActor *a)
{
    return g_btl_key_clut_first[a->c.key];
}
