/* Persona 1 (JP) - what the battle's first sequence is playing at.
 *   BTLP @ 0x80066C7C
 *
 * SsSeqGetVol hands back the two channels separately; every caller wants one
 * number, so this is where they are averaged. Sequence bank 0 is the one the
 * battle opens first and the only one asked about.
 */
#include <types.h>
#include <libsnd.h>

extern short g_btl_seq[];

short BtlSeqVolumeMean(void)
{
    short voll;
    short volr;

    SsSeqGetVol(g_btl_seq[0], 0, &voll, &volr);
    return (voll + volr) / 2;
}
