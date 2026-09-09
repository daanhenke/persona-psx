/* Persona 1 (JP) - a demon's voice bank.  BTLP only.
 *   0x80091F74 BtlLoadVoiceBank  0x80092058 BtlOpenVoiceBank
 *   0x80092004 BtlReadVoiceBank  0x800920AC BtlWaitVabTrans
 *
 * Four steps of one job, and callers use whichever half they need: the load
 * reads a bank and opens it in one breath, the read only starts the transfer
 * and comes back, the open hands what has arrived to the SPU, and the wait
 * holds for the SPU to take the body across while frames keep being drawn.
 *
 * A bank goes into slot 4, the same one the battle music uses, so a voice and
 * the music cannot both be resident.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/battle.h>

void BtlLoadVoiceBank(int entry)
{
    BtlSoundBank bank;

    entry += BTL_VOICE_FIRST;
    BtlReadSectors((u_long *)BTL_VOICE_BUFFER,
                   g_btl_pack_sectors[entry] + g_btl_voice_base,
                   g_btl_pack_sectors[entry + 1] - g_btl_pack_sectors[entry]);
    bank.nsep = BTL_VOICE_SEPS;
    bank.vb = g_btl_voice_vb;
    bank.vh = g_btl_voice_vh;
    bank.seq = g_btl_voice_seq;
    BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
}

void BtlReadVoiceBank(int entry)
{
    /* Sixteen bytes of frame nothing here uses; the function is the wrong
       length without them. */
    u_long header[4];

    entry += BTL_VOICE_FIRST;
    BtlReadSectorsAsync((u_long *)BTL_VOICE_BUFFER,
                        g_btl_pack_sectors[entry] + g_btl_voice_base,
                        g_btl_pack_sectors[entry + 1]
                            - g_btl_pack_sectors[entry]);
}

void BtlOpenVoiceBank(void)
{
    BtlSoundBank bank;

    bank.nsep = BTL_VOICE_SEPS;
    bank.vb = g_btl_voice_vb;
    bank.vh = g_btl_voice_vh;
    bank.seq = g_btl_voice_seq;
    BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
}

void BtlWaitVabTrans(void)
{
    while (SsVabTransCompleted(0) == 0) {
        BtlDrawFrame();
    }
}
