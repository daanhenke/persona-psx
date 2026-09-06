/* Persona 1 (JP) - the acting character's turn in a negotiation.  BTLP only.
 *   0x8006A22C BtlTalkPerform
 *
 * Run for one verb only - the one scored off the mood band rather than the
 * line's weight - and it is the longest thing the negotiation does. A pack
 * entry is read off the disc and played on the BGM slot, the animation
 * sequencer runs for two seconds under it, and then the character's own bank
 * is opened and played while their object goes to motion 7.
 *
 * The character's bank is found by their Char key, so the same verb sounds
 * different for each of them.
 */
#include <types.h>
#include <libsnd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/sound.h>

/* The pack entry this plays, and how long the sequencer runs under it. */
#define TALK_PERFORM_PACK   10
#define TALK_PERFORM_FRAMES 0x78

/* The slot the character's own bank is opened in, and the motion it puts
   them on. */
#define TALK_VOICE_SLOT   6
#define TALK_PERFORM_STEP 7

/* Where the sequencer is left, and how long that takes. */
#define TALK_SEQ_STATE 8
#define TALK_SEQ_TIME  4

/* A last beat before handing control back. */
#define TALK_TAIL_FRAMES 5

extern BtlActor            g_btl_actors[];
extern short               g_btl_actor_slot;
extern const BtlSoundBank  g_btl_banks[];
extern volatile long       g_cd_busy;

/* Two sequence handles four bytes apart. seq.c reaches the first as a scalar;
   this one wants the second, so they are indexed here. */
extern short g_btl_seq_handle[];

extern void BtlLoadPackEntry(int entry);
extern void BtlBgmOpen(void);
extern void BtlDrawFrame(void);
extern void BtlSeqEndIfDone(void);
extern void BtlSeqWaitDone(void);
extern void BtlSeqSetState(int state, int frames);
extern void BtlIndicatorBar(void);
extern void BtlRunFrames(int frames);
extern void BtlObjSetMotion(BtlObj *obj, u_char motion);

void BtlTalkPerform(void)
{
    BtlActor *a;
    int       i;

    i = 0;
    a = &g_btl_actors[g_btl_actor_slot];
    BtlLoadPackEntry(TALK_PERFORM_PACK);
    while (g_cd_busy != -1) {
        BtlDrawFrame();
    }
    BtlBgmOpen();
    SsVabTransCompleted(1);
    BtlSePlay(BTL_BGM_SLOT, 0);
    do {
        BtlSeqEndIfDone();
        i++;
        BtlDrawFrame();
    } while (i < TALK_PERFORM_FRAMES);
    BtlSoundClose(BTL_BGM_SLOT);

    BtlSoundOpen(g_btl_banks, TALK_VOICE_SLOT, a->c.key);
    BtlDrawFrame();
    BtlSePlay(TALK_VOICE_SLOT, 0);
    BtlObjSetMotion(a->obj, TALK_PERFORM_STEP);
    SsSepStop(g_btl_seq_handle[2], 0);
    BtlSeqWaitDone();
    BtlSoundClose(TALK_VOICE_SLOT);

    BtlSeqSetState(TALK_SEQ_STATE, TALK_SEQ_TIME);
    BtlIndicatorBar();
    BtlRunFrames(TALK_TAIL_FRAMES);
}
