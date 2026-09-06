/* Persona 1 (JP) - handing the music to the battle and back.
 *   BTLP @ 0x80092DD8 BtlBgmOpen, 0x80066EF8 BtlBgmRestore
 *
 * The battle's own music arrives as a pack entry read to 0x80152400, whose
 * first three words are the VAB body, the VAB header and the SEQ. BtlBgmOpen
 * wraps them in a bank and opens it as slot 4.
 *
 * Going back the other way is not just a close: the SPU needs a moment before
 * the field sequence starts again, so thirty frames go by first, and the field
 * music fades in rather than cutting.
 */
#include <types.h>
#include <libsnd.h>
#include <persona/btlp/sound.h>

/* Where BtlLoadPackEntry leaves whatever it read. */
#define BTL_PACK_DEST 0x80152400

#define g_btl_bgm_vb  (*(u_char **)(BTL_PACK_DEST + 0x0))
#define g_btl_bgm_vh  (*(u_char **)(BTL_PACK_DEST + 0x4))
#define g_btl_bgm_seq (*(u_long **)(BTL_PACK_DEST + 0x8))

#define BTL_BGM_SEPS 4

/* Frames of quiet between closing the battle bank and restarting the field
   sequence, and the volume and time the field music fades back up over. */
#define BGM_GAP_FRAMES 30
#define BGM_BACK_VOL   60
#define BGM_BACK_TIME  15

extern int g_btl_bgm_state;

extern void BtlRunFrames(int frames);

void BtlBgmOpen(void)
{
    BtlSoundBank bank;

    bank.nsep = BTL_BGM_SEPS;
    bank.vb = g_btl_bgm_vb;
    bank.vh = g_btl_bgm_vh;
    bank.seq = g_btl_bgm_seq;
    BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
}

void BtlBgmRestore(void)
{
    if (g_btl_bgm_state != 0) {
        BtlSoundClose(BTL_BGM_SLOT);
        BtlRunFrames(BGM_GAP_FRAMES);
        SsSepPlay(g_btl_seq[0], 0, SSPLAY_PLAY, 1);
        SsSepSetCrescendo(g_btl_seq[0], 0, BGM_BACK_VOL, BGM_BACK_TIME);
    }
}
