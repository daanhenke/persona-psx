/* Persona 1 (JP) - handing the music back to the field.
 *   BTLP @ 0x80066EF8 BtlBgmRestore
 *
 * Not just a close: the SPU needs a moment before the field sequence starts
 * again, so thirty frames go by first, and the field music fades in rather
 * than cutting.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/battle.h>

/* Frames of quiet between closing the battle bank and restarting the field
   sequence, and the volume and time the field music fades back up over. */
#define BGM_GAP_FRAMES 30
#define BGM_BACK_VOL   60
#define BGM_BACK_TIME  15

extern int g_btl_bgm_state;

void BtlBgmRestore(void)
{
    if (g_btl_bgm_state != 0) {
        BtlSoundClose(BTL_BGM_SLOT);
        BtlRunFrames(BGM_GAP_FRAMES);
        SsSepPlay(g_btl_seq[0], 0, SSPLAY_PLAY, 1);
        SsSepSetCrescendo(g_btl_seq[0], 0, BGM_BACK_VOL, BGM_BACK_TIME);
    }
}
