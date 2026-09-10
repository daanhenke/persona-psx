/* Persona 1 (JP) - swapping the battle's music.
 *   BTLP @ 0x80066D18 BtlBgmChange
 *
 * The old sequence is faded down while the new pack entry is read off the
 * disc, and the read is waited out here rather than by a callback, so the
 * screen keeps drawing throughout.
 *
 * Opening the bank lives in packopen.c and restoring the field music in
 * bgmrestore.c: the image puts all three in different places.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/pack.h>
#include <persona/main/cd.h>
#include <persona/btlp/battle.h>

/* One byte per (track, column). 0xFF means play nothing; otherwise the low
   nibble is the sequence inside the bank, and the two top bits say how to read
   it. */
#define BGM_SILENT   0xFF
#define BGM_SEQ      0x0F
#define BGM_ABSOLUTE 0x40 /* the nibble stands alone, not as an offset */
#define BGM_ONESHOT  0x80 /* the track ends; BtlWaitBgmEnd waits for it  */

/* BtlWaitBgmEnd tests the sign of g_btl_bgm_seq, so BGM_ONESHOT is carried in
   the top bit of the number and masked off again when it is played. */
#define BGM_ENDS 0x80000000

/* Frames of quiet between closing the battle bank and restarting the field
   sequence, and the volume and time the field music fades back up over. */
#define BGM_GAP_FRAMES 30
#define BGM_BACK_VOL   60
#define BGM_BACK_TIME  15

void BtlBgmChange(int track, int column, int base)
{
    u_char code;
    int    seq;

    code = g_btl_bgm_table[track][column];

    if (code != BGM_SILENT)
    {
        if ((code & BGM_ABSOLUTE) == 0)
        {
            g_btl_bgm_seq = base + (code & BGM_SEQ);
        }
        else
        {
            g_btl_bgm_seq = code & BGM_SEQ;
        }
        seq = 0;
        if ((code & BGM_ONESHOT) != 0)
        {
            g_btl_bgm_seq = g_btl_bgm_seq | BGM_ENDS;
        }
        SsSepSetDecrescendo(g_btl_seq[0], seq, BGM_BACK_VOL, BGM_BACK_TIME);
        BtlLoadPackEntry(track);
        while (g_cd_busy != -1)
        {
            BtlDrawFrame();
        }
        BtlBgmOpen();
        BtlRunFrames(BGM_GAP_FRAMES);
        SsSepStop(g_btl_seq[0], 0);
        SsVabTransCompleted(1);
        BtlSePlay(BTL_BGM_SLOT, g_btl_bgm_seq & ~BGM_ENDS);
        g_btl_bgm_state = 1;
    }
    else
    {
        BtlRunFrames(BGM_GAP_FRAMES);
        g_btl_bgm_state = 0;
    }
}
