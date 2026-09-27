/* Persona 1 (JP) - S2D's sound: bringing libsnd up, opening the menu and map
 * banks' sequences, and switching the background music.
 *
 * Sequence handles live in the save area's table (g_seq_handle), fixed slots:
 * 0-2 the map's own sequences, 3 and 7-15 the menu sounds, 16 the music now
 * playing, 17 the one fading out under it, 18-19 two more menu sounds. The
 * sequences all come out of one pack read to 0x801D8000, whose first words
 * are the offsets of its entries.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <libspu.h>
#include <memory.h>
#include <persona/main/state.h>

#define g_vab_id     ((short *)0x801F535C)
#define g_seq_handle ((short *)0x801F537C)
#define SEQ_PACK     ((u_long *)0x801D8000)
#define PACK_AT(i)   ((u_long *)((u_char *)SEQ_PACK + SEQ_PACK[i]))

/* The menu VAB: its header is carried in S2D's own data and copied here
   before SsVabOpenHead; the body comes with the LTS file at 0x800D0000,
   whose fourth word is its offset. */
#define MENU_VAB_HEAD ((u_char *)0x801D5000)
#define LTS_FILE      ((u_long *)0x800D0000)

#define SEQ_BGM      16
#define SEQ_BGM_OUT  17

extern short  g_bgm_ready;
extern u_char g_btl_map_id;
extern u_char g_menu_vab_head[];
extern int    g_cur_bgm;
extern int    g_mark_maps[8];
extern int    g_mark_on;
extern u_char g_options[];

extern void DngSeqMarkCallback(short access, short seq, short data);

void SoundSetOutput(void);
void SoundPlayBgm(int track);

void SoundInit(void)
{
    int i;

    SpuSetTransferStartAddr(0x1010);
    SpuWrite0(0x70000);
    SsSetMVol(0, 0);
    SsSetRVol(0, 0);
    SsSetMono();
    SoundSetOutput();
    SsSetTableSize((char *)0x801F0000, 0x14, 1);
    SsSetTickMode(SS_TICK60);
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
    SsSetRVol(0x7F, 0x7F);
    SsUtSetReverbDepth(0x40, 0x40);
    SsUtReverbOn();
    SsSetMVol(0x7F, 0x7F);
    SsStart();
    {
        short none = -1;

        for (i = 19; i >= 0; i--) {
            g_seq_handle[i] = none;
        }
    }
}

/* Mono or stereo, as the options screen has it. */
void SoundSetOutput(void)
{
    if (g_options[0] != 0) {
        SsSetStereo();
    } else {
        SsSetMono();
    }
}

void SoundOpenMapSeqs(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        g_seq_handle[i] = SsSeqOpen(PACK_AT(i + 1), g_vab_id[0]);
    }
}

/* The menu bank, unless ADV has left it loaded; then the music. */
void SoundOpenMenuBank(void)
{
    u_char *body;

    body = (u_char *)(LTS_FILE[3] + (u_long)LTS_FILE);
    if (g_state_prev != GAME_STATE_ADV || g_bgm_ready == 1) {
        memmove(MENU_VAB_HEAD, g_menu_vab_head, 0x2C20);
        do {
            g_vab_id[1] = SsVabOpenHead(MENU_VAB_HEAD, -1);
        } while (g_vab_id[1] == -1);
        SsVabTransBody(body, g_vab_id[1]);
        SsVabTransCompleted(1);
        g_seq_handle[3] = SsSeqOpen(PACK_AT(12), g_vab_id[1]);
        g_seq_handle[8] = SsSeqOpen(PACK_AT(5), g_vab_id[1]);
        g_seq_handle[9] = SsSeqOpen(PACK_AT(6), g_vab_id[1]);
        g_seq_handle[7] = SsSeqOpen(PACK_AT(7), g_vab_id[1]);
        g_seq_handle[13] = SsSeqOpen(PACK_AT(13), g_vab_id[1]);
        g_seq_handle[14] = SsSeqOpen(PACK_AT(14), g_vab_id[1]);
        g_seq_handle[15] = SsSeqOpen(PACK_AT(15), g_vab_id[1]);
        g_seq_handle[10] = SsSeqOpen(PACK_AT(25), g_vab_id[1]);
        g_seq_handle[11] = SsSeqOpen(PACK_AT(25), g_vab_id[1]);
        g_seq_handle[12] = SsSeqOpen(PACK_AT(11), g_vab_id[1]);
    }
    g_seq_handle[18] = SsSeqOpen(PACK_AT(23), g_vab_id[1]);
    g_seq_handle[19] = SsSeqOpen(PACK_AT(24), g_vab_id[1]);
    if (g_btl_map_id != 0) {
        if (g_seq_handle[SEQ_BGM] != -1) {
            SsSeqStop(g_seq_handle[SEQ_BGM]);
            SsSetNck(g_seq_handle[SEQ_BGM]);
            g_seq_handle[SEQ_BGM] = -1;
        }
    } else {
        SoundPlayBgm(0);
    }
}

/* Fades the music now playing out under the new one. */
void SoundPlayBgm(int track)
{
    u_char tracks[7] = { 0x14, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x15 };
    int    seq;

    if (g_cur_bgm != track) {
        seq = tracks[track];
        if (g_seq_handle[SEQ_BGM_OUT] != -1) {
            SsSeqStop(g_seq_handle[SEQ_BGM_OUT]);
            SsSetNck(g_seq_handle[SEQ_BGM_OUT]);
            g_seq_handle[SEQ_BGM_OUT] = -1;
        }
        if (g_seq_handle[SEQ_BGM] != -1) {
            g_seq_handle[SEQ_BGM_OUT] = g_seq_handle[SEQ_BGM];
            SsSeqSetDecrescendo(g_seq_handle[SEQ_BGM], 0x7E, 0x1E);
        }
        g_seq_handle[SEQ_BGM] = SsSeqOpen(PACK_AT(seq + 1), g_vab_id[1]);
        SsSeqSetVol(g_seq_handle[SEQ_BGM], 0x7E, 0x7E);
        SsSeqPlay(g_seq_handle[SEQ_BGM], 1, 1);
        g_cur_bgm = track;
    }
}

#define SEQ_KILL(n)                        \
    if (g_seq_handle[n] != -1) {           \
        SsSeqStop(g_seq_handle[n]);        \
        SsSetNck(g_seq_handle[n]);         \
        g_seq_handle[n] = -1;              \
    }

void SoundStopBgm(void)
{
    SEQ_KILL(16);
    SEQ_KILL(17);
    SEQ_KILL(18);
    SEQ_KILL(19);
}

/* Some maps hand their music between sequences on a mark. */
void SoundCheckMarkMap(int map)
{
    int i;

    g_mark_on = 0;
    for (i = 0; i < 8; i++) {
        if (g_mark_maps[i] == map) {
            g_mark_on = 1;
            break;
        }
    }
}

void SoundArmMark(void)
{
    if (g_mark_on == 1) {
        SsSetMarkCallback(g_seq_handle[0], 0, (SsMarkCallbackProc)DngSeqMarkCallback);
    }
    g_btl_map_id = g_mark_on;
}

void SoundLoadFxA(void)
{
    SsSeqStop(g_seq_handle[10]);
    SsSeqStop(g_seq_handle[11]);
    SsSeqStop(g_seq_handle[12]);
    SsSetNck(g_seq_handle[10]);
    SsSetNck(g_seq_handle[11]);
    SsSetNck(g_seq_handle[12]);
    g_seq_handle[10] = SsSeqOpen(PACK_AT(25), g_vab_id[1]);
    g_seq_handle[11] = SsSeqOpen(PACK_AT(25), g_vab_id[1]);
    g_seq_handle[12] = SsSeqOpen(PACK_AT(11), g_vab_id[1]);
}

void SoundLoadFxB(void)
{
    SsSeqStop(g_seq_handle[10]);
    SsSeqStop(g_seq_handle[11]);
    SsSeqStop(g_seq_handle[12]);
    SsSetNck(g_seq_handle[10]);
    SsSetNck(g_seq_handle[11]);
    SsSetNck(g_seq_handle[12]);
    g_seq_handle[10] = SsSeqOpen(PACK_AT(8), g_vab_id[1]);
    g_seq_handle[11] = SsSeqOpen(PACK_AT(9), g_vab_id[1]);
    g_seq_handle[12] = SsSeqOpen(PACK_AT(10), g_vab_id[1]);
}

/* A byte of padding after the track table that the image carries as 0x80. */
const u_char soundsetup_pad = 0x80;
