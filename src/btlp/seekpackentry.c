/* Persona 1 (JP) - pointing the drive at a pack entry without reading it.
 *   BTLP @ 0x80066CCC BtlSeekPackEntry
 *
 * The seek is issued early so it is over by the time the read is asked for.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

extern u_short g_btl_pack_offsets[];
extern int     g_btl_pack_base;

void BtlSeekPackEntry(int entry)
{
    CdlLOC loc;

    CdIntToPos(g_btl_pack_offsets[entry] + g_btl_pack_base, &loc);
    CdControl(CdlSeekL, (u_char *)&loc, 0);
}
