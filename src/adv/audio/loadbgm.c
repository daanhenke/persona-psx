/* Persona 1 (JP) - starting a scene's music.  ADV only.
 *   0x80088BEC AdvLoadBgm
 *
 * Music is two files: BGM.BIN entry `id` is a small pack of VAB headers and
 * sequences, read to 0x801CD000, and BVB.BIN entry `id` is the matching VAB
 * body. Both are streamed through AdvSelectFile, with the reads eight bytes
 * early so the eight-byte entry header lands in front of the buffer. A second
 * VAB, BVB entry 0x3F, is shared by every scene and takes the pack's second
 * header; its sequence is opened into seq slot 10 for the jingles.
 *
 * Loading tears libsnd down and brings it back up first - output mode from
 * the MONO / STEREO option, reverb on - and afterwards marks every SE bank
 * slot empty, since the sound RAM they pointed into has been reused.
 *
 * Two ids are commands instead: 0xFE fades the music out, 0xFF stops it.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <libcd.h>
#include <persona/main/cd.h>

#define BGM_FIRST_COMMAND 0x40
#define BGM_FADE_OUT      0xFE
#define BGM_STOP          0xFF

#define FILE_BGM 7
#define FILE_BVB 8
#define BVB_SHARED 0x3F

/* The music pack, as byte offsets from its own start: the scene's VAB
   header, the shared VAB's header, the scene's sequence, and at +0x14 the
   jingle sequence. */
#define g_bgm_pack    ((u_long *)0x801CD000)
#define BGM_VAB       0
#define BGM_VAB_SHARED 1
#define BGM_SEQ       2
#define BGM_SEQ_JINGLE 5
#define PACK_AT(i)    ((u_char *)g_bgm_pack + g_bgm_pack[i])

/* VAB bodies are staged here before the transfer to sound RAM. */
#define VAB_BODY_STAGE 0x80118000
#define ENTRY_HEADER   8

/* All reached by hardcoded address, as in sound.c. */
#define g_vab_id     ((short *)0x801F535C)
#define g_seq_handle ((short *)0x801F537C)
#define g_bank_seq   ((short *)0x801F539E)
#define SEQ_BGM      0
#define SEQ_JINGLE   10
#define BANK_SLOTS   6

typedef struct {
    u_short file_id;
    u_short vab_id;
} AdvBankHead;

extern AdvBankHead *g_adv_banks[];
extern u_char       g_options[];     /* [0]: stereo */
extern short        g_bgm_ready;

extern CdlFILE      g_adv_scene_file;
extern volatile int g_cd_busy;

extern void AdvSelectFile(short kind, short id);
extern void AdvRunFrame(void);
/* Declared here with a wide first parameter: the handle is passed unmasked. */
extern void SoundFadeOutSeq(short slot, u_char vol, short time, short frames);

void AdvLoadBgm(short id)
{
    AdvBankHead *bank;
    int          i;

    if (id < BGM_FIRST_COMMAND) {
        SsEnd();
        SsQuit();
        SsInit();
        SsSetMVol(0, 0);
        SsSetTableSize((char *)0x801F0000, 0x20, 1);
        SsSetTickMode(SS_TICK60);
        if (g_options[0] != 0) {
            SsSetStereo();
        } else {
            SsSetMono();
        }
        SsStart2();
        SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
        SsUtSetReverbDepth(0x40, 0x40);
        SsUtReverbOn();

        AdvSelectFile(FILE_BGM, id);
        CdReadFileToAddrAsync(&g_adv_scene_file, g_adv_scene_file.size,
                              (u_long *)((u_char *)g_bgm_pack - ENTRY_HEADER));
        while (g_cd_busy != -1) {
            AdvRunFrame();
        }
        g_vab_id[0] = SsVabOpenHead(PACK_AT(BGM_VAB), -1);

        AdvSelectFile(FILE_BVB, id);
        CdReadFileToAddrAsync(&g_adv_scene_file, g_adv_scene_file.size,
                              (u_long *)(VAB_BODY_STAGE - ENTRY_HEADER));
        while (g_cd_busy != -1) {
            AdvRunFrame();
        }
        SsVabTransBody((u_char *)VAB_BODY_STAGE, g_vab_id[0]);
        while (SsVabTransCompleted(SS_IMEDIATE) == 0) {
            AdvRunFrame();
        }

        g_vab_id[1] = SsVabOpenHead(PACK_AT(BGM_VAB_SHARED), -1);
        AdvSelectFile(FILE_BVB, BVB_SHARED);
        CdReadFileToAddrAsync(&g_adv_scene_file, g_adv_scene_file.size,
                              (u_long *)(VAB_BODY_STAGE - ENTRY_HEADER));
        while (g_cd_busy != -1) {
            AdvRunFrame();
        }
        SsVabTransBody((u_char *)VAB_BODY_STAGE, g_vab_id[1]);
        while (SsVabTransCompleted(SS_IMEDIATE) == 0) {
            AdvRunFrame();
        }

        SsSetMVol(0x7F, 0x7F);
        g_seq_handle[SEQ_BGM] =
            SsSeqOpen((u_long *)PACK_AT(BGM_SEQ), g_vab_id[0]);
        SsSeqSetVol(g_seq_handle[SEQ_BGM], 0x7F, 0x7F);
        SsSeqPlay(g_seq_handle[SEQ_BGM], SSPLAY_PLAY, 1);
        g_seq_handle[SEQ_JINGLE] =
            SsSeqOpen((u_long *)PACK_AT(BGM_SEQ_JINGLE), g_vab_id[1]);
        g_bgm_ready = 1;

        for (i = 0; i < BANK_SLOTS; i++) {
            bank = g_adv_banks[i];
            bank->file_id = 0xFFFF;
            bank->vab_id = 0xFFFF;
            g_bank_seq[i] = -1;
        }
        return;
    }

    switch (id) {
    case BGM_FADE_OUT:
        /* The handle goes where SoundFadeOutSeq wants a slot number; it only
           works while the music has handle 0. */
        /* The handle goes where SoundFadeOutSeq wants a slot number; it only
           works while the music holds handle 0. */
        SoundFadeOutSeq(g_seq_handle[SEQ_BGM], 0x7F, 0x80, 0x80);
        break;
    case BGM_STOP:
        SsSeqStop(g_seq_handle[SEQ_BGM]);
        break;
    }
}
