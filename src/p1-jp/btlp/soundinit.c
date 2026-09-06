/* Persona 1 (JP) - restarting the sound driver for a battle.
 *   BTLP @ 0x80081A94
 *
 * The overlay does not inherit the field's sound state: it turns reverb off,
 * shuts the driver down and brings it back up, so the SEQ and SEP tables are
 * its own. Mono or stereo follows the config screen's SOUND row.
 *
 * All sixteen bank slots are then marked free, which is a negative id in both
 * the VAB and the SEQ array.
 */
#include <types.h>
#include <libsnd.h>
#include <persona/btlp/sound.h>

/* Twelve SEQs and twelve SEPs is all a battle needs. */
#define BTL_SEQ_MAX 12
#define BTL_SEP_MAX 12

#define BTL_MVOL 0x7F

extern u_char g_options[];
extern char   g_btl_seq_table[];

void BtlSoundInit(void)
{
    short *vab;
    short *seq;
    int    i;
    short  free;

    SsUtReverbOff();
    SsUtSetReverbType(SS_REV_TYPE_OFF);
    SsUtSetReverbDepth(0, 0);
    SsEnd();
    SsQuit();
    SsInit();
    if (g_options[0] != 0) {
        SsSetStereo();
    } else {
        SsSetMono();
    }
    SsSetTickMode(SS_TICK60);
    SsSetTableSize(g_btl_seq_table, BTL_SEQ_MAX, BTL_SEP_MAX);
    SsSetMVol(BTL_MVOL, BTL_MVOL);
    SsStart();

    /* The free marker is a local so it is materialised before the two
       pointers, which is the order the original sets the loop up in. */
    i = 0;
    free = -1;
    vab = g_btl_vab;
    seq = g_btl_seq;
    do {
        *seq = free;
        *vab = free;
        vab++;
        g_btl_seq_count[i] = 0;
        i++;
        seq++;
    } while (i < BTL_SOUND_SLOTS);
}
