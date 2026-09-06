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

/* The head of whatever BtlLoadPackEntry last read. */
extern u_char *g_btl_pack_vb;
extern u_char *g_btl_pack_vh;
extern u_long *g_btl_pack_seq;

#define BTL_BGM_SEPS 4

/* Frames of quiet between closing the battle bank and restarting the field
   sequence, and the volume and time the field music fades back up over. */
#define BGM_GAP_FRAMES 30
#define BGM_BACK_VOL   60
#define BGM_BACK_TIME  15

/* One byte per (track, column). 0xFF means play nothing; otherwise the low
   nibble is the sequence inside the bank, and the two top bits say how to read
   it. */
#define BGM_SILENT   0xFF
#define BGM_SEQ      0x0F
#define BGM_ABSOLUTE 0x40   /* the nibble stands alone, not as an offset */
#define BGM_ONESHOT  0x80   /* the track ends; BtlWaitBgmEnd waits for it  */

/* BtlWaitBgmEnd tests the sign of g_btl_bgm_seq, so BGM_ONESHOT is carried in
   the top bit of the number and masked off again when it is played. */
#define BGM_ENDS 0x80000000

extern int g_btl_bgm_state;
extern int g_btl_bgm_seq;
extern u_char g_btl_bgm_table[][4];

extern void BtlRunFrames(int frames);
extern void BtlLoadPackEntry(int entry);
extern void BtlDrawFrame(void);
extern volatile int g_cd_busy;

/* No prototype: this file hands BtlSePlay a whole word and lets the callee
   truncate it, which is what keeps the mask below a 32-bit one. */
extern void BtlSePlay();

void BtlBgmOpen(void)
{
    BtlSoundBank bank;

    bank.nsep = BTL_BGM_SEPS;
    bank.vb = g_btl_pack_vb;
    bank.vh = g_btl_pack_vh;
    bank.seq = g_btl_pack_seq;
    BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
}

/* Swapping the battle's music. The old sequence is faded down while the new
   pack entry is read off the disc, and the read is waited out here rather than
   by a callback, so the screen keeps drawing throughout. */
void BtlBgmChange(int track, int column, int base)
{
    u_char code;
    int    seq;

    code = g_btl_bgm_table[track][column];
    if (code != BGM_SILENT) {
        seq = code & BGM_SEQ;
        if ((code & BGM_ABSOLUTE) == 0) {
            seq = seq + base;
        }
        g_btl_bgm_seq = seq;
        if ((code & BGM_ONESHOT) != 0) {
            g_btl_bgm_seq = g_btl_bgm_seq | BGM_ENDS;
        }
        SsSepSetDecrescendo(g_btl_seq[0], 0, BGM_BACK_VOL, BGM_BACK_TIME);
        BtlLoadPackEntry(track);
        while (g_cd_busy != -1) {
            BtlDrawFrame();
        }
        BtlBgmOpen();
        BtlRunFrames(BGM_GAP_FRAMES);
        SsSepStop(g_btl_seq[0], 0);
        SsVabTransCompleted(1);
        BtlSePlay(BTL_BGM_SLOT, g_btl_bgm_seq & ~BGM_ENDS);
        g_btl_bgm_state = 1;
    } else {
        BtlRunFrames(BGM_GAP_FRAMES);
        g_btl_bgm_state = 0;
    }
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
