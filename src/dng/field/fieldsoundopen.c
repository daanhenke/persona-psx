/* Persona 1 (JP) - opening the floor's sound.  DNG only.
 *   0x8006EAAC FieldOpenSound
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/main/state.h>
#include <persona/dng/field.h>

/* The save's sound option: set for stereo. */
extern u_char g_cfg_stereo;

/* Bit 1: on map 9, the ambient sequence plays. */
extern u_char D_801F29ED;

#define SEQ(n) ((u_long *)(SEQ_BASE + SEQ_OFFSETS[n]))

/* The handles the floor's tunes and the idle tune play in. */
#define TUNE_A    13
#define TUNE_B    14
#define TUNE_BGM  15
#define TUNE_IDLE 16


/* Opens the floor's sequences - unless an ADV scene left them open, libsnd
   is set up first and the ambient and effect sequences opened too - sets
   every open one to full volume, starts the floor's two tunes and the
   ambient as the map and the floor ask, and starts the walking tune. */
void FieldOpenSound(void)
{
    int i;

    if (!SOUND_KEPT) {
        if (g_cfg_stereo) {
            SsSetStereo();
        } else {
            SsSetMono();
        }
        SsUtSetReverbType(4);
        SsUtReverbOn();
        SsSetTickMode(1);
        SsSetTableSize((char *)g_dng, 32, 1);
        g_seq_handles[0] = SsSeqOpen(SEQ(0), g_vab_handles[0]);
        if (SEQ_OFFSETS[1] != 0) {
            g_seq_handles[1] = SsSeqOpen(SEQ(1), g_vab_handles[0]);
        }
        if (SEQ_OFFSETS[2] != 0) {
            g_seq_handles[2] = SsSeqOpen(SEQ(2), g_vab_handles[0]);
        }
        g_seq_handles[3] = SsSeqOpen(SEQ(4), g_vab_handles[1]);
        g_seq_handles[4] = SsSeqOpen(SEQ(5), g_vab_handles[1]);
        g_seq_handles[5] = SsSeqOpen(SEQ(6), g_vab_handles[1]);
        g_seq_handles[6] = SsSeqOpen(SEQ(4), g_vab_handles[1]);
        g_seq_handles[7] = SsSeqOpen(SEQ(5), g_vab_handles[1]);
        g_seq_handles[8] = SsSeqOpen(SEQ(6), g_vab_handles[1]);
        g_seq_handles[10] = SsSeqOpen(SEQ(14), g_vab_handles[1]);
        g_seq_handles[11] = SsSeqOpen(SEQ(17), g_vab_handles[1]);
        SsStart();
        SsSetMVol(0x7F, 0x7F);
        VSync(60);
        SsUtSetReverbDepth(0x40, 0x40);
    }
    g_seq_handles[12] = SsSeqOpen(SEQ(11), g_vab_handles[1]);
    g_seq_handles[18] = SsSeqOpen(SEQ(3), g_vab_handles[1]);
    if (SEQ_OFFSETS[22] != 0) {
        g_seq_handles[TUNE_A] = SsSeqOpen(SEQ(22), g_vab_handles[2]);
    }
    if (SEQ_OFFSETS[23] != 0) {
        g_seq_handles[TUNE_B] = SsSeqOpen(SEQ(23), g_vab_handles[2]);
    }
    g_seq_handles[TUNE_IDLE] = SsSeqOpen(SEQ(13), g_vab_handles[1]);
    g_seq_handles[TUNE_BGM] = SsSeqOpen(SEQ(12), g_vab_handles[1]);
    g_seq_handles[17] = SsSeqOpen(SEQ(3), g_vab_handles[2]);
    for (i = 0; i < 18; i++) {
        if (g_seq_handles[i] != -1) {
            SsSeqSetVol(g_seq_handles[i], 0x7F, 0x7F);
        }
    }

    if (g_map_music[g_dng->map][0] != 2 && g_seq_handles[TUNE_A] != -1
        && (g_dng->map != 9 || (D_801F29ED & 2))
        && !((g_dng->map == 1 || g_dng->map == 5) && (g_quest_bits & 0x10))) {
        if (g_map_music[g_dng->map][0] == 1) {
            SsSeqSetVol(g_seq_handles[TUNE_A], 0, 0);
        }
        if (g_dng->map != 28 && !g_clock_on) {
            SsSeqPlay(g_seq_handles[TUNE_A], 1, 0);
        }
    }
    if (g_map_music[g_dng->map][1] != 2 && g_seq_handles[TUNE_B] != -1) {
        if (g_map_music[g_dng->map][1] == 1) {
            SsSeqSetVol(g_seq_handles[TUNE_B], 0, 0);
        }
        if (g_dng->map != 28 || g_clock_on) {
            SsSeqPlay(g_seq_handles[TUNE_B], 1, 0);
        }
    }

    if (g_dng->map == 9) {
        if (D_801F29ED & 2) {
            if (g_ambient_seq == -1) {
                g_ambient_seq = SsSeqOpen(SEQ(0), g_vab_handles[0]);
            }
            SsSeqPlay(g_ambient_seq, 1, 0);
        } else if (g_ambient_seq != -1) {
            SsSetNck(g_ambient_seq);
            g_ambient_seq = -1;
        }
        g_ambient_on = 1;
    } else if (!SOUND_KEPT) {
        SsSeqPlay(g_ambient_seq, 1, 0);
        if (FLOOR_FLAGS & FLOOR_QUIET) {
            SsSeqSetVol(g_ambient_seq, 0, 0);
            g_ambient_on = 0;
        } else {
            g_ambient_on = 1;
        }
    }
    FieldSyncMusic(1);
    g_bgm_flags = 0;
    g_idle_seq = g_seq_handles[TUNE_IDLE];
    g_bgm_seq = g_seq_handles[TUNE_BGM];
    FieldZoneTunes(1);
    SsSeqPlay(g_bgm_seq, 0, 0);
}
