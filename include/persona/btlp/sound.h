#ifndef PERSONA_BTLP_SOUND_H
#define PERSONA_BTLP_SOUND_H

/* Persona 1 (JP) - the battle overlay's sound banks.
 *
 * Up to sixteen banks are open at once, each a VAB plus the SEQ that plays
 * against it. A bank is described on disc by four words, and the three parallel
 * arrays below hold what the SPU handed back for each open slot; a free slot is
 * marked by a negative id in both. Slot 4 is the battle BGM, which is why
 * BtlBgmOpen and BtlBgmRestore name it directly.
 */
#include <types.h>

typedef struct {
    /* 0x0 */ u_char *vh;           /* VAB header */
    /* 0x4 */ u_char *vb;           /* VAB body, pushed to SPU RAM */
    /* 0x8 */ u_long *seq;          /* SEQ data */
    /* 0xC */ short   nsep;         /* sub-sequences in the SEQ */
} BtlSoundBank;                     /* 0x10 bytes */

#define BTL_SOUND_SLOTS 16
#define BTL_BGM_SLOT    4

extern short  g_btl_vab[];
extern short  g_btl_seq[];
extern u_char g_btl_seq_count[];

extern int  BtlSoundOpen(const BtlSoundBank *banks, int slot, int index);
extern void BtlSoundClose(int slot);

/* BtlSePlay takes its sequence as a short, but not every caller knows that:
   several translation units declare it without a prototype and hand it a full
   word, which the callee simply truncates. Its declaration therefore belongs
   with each caller rather than here. */

#endif
