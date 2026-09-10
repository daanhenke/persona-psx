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
#include <decomp/types.h>

typedef struct {
    /* 0x0 */ u_char *vh;           /* VAB header */
    /* 0x4 */ u_char *vb;           /* VAB body, pushed to SPU RAM */
    /* 0x8 */ u_long *seq;          /* SEQ data */
    /* 0xC */ short   nsep;         /* sub-sequences in the SEQ */
} BtlSoundBank;                     /* 0x10 bytes */

#define BTL_SOUND_SLOTS 16
#define BTL_BGM_SLOT    4

/* The demon voice banks. They share one table of start sectors with the rest
   of the sound pack - the voices are simply the entries from BTL_VOICE_FIRST
   on, which is why the loaders index it that far in. An entry runs to where
   the next one starts, so a length is always a subtraction.

   splat cuts that table into two symbols at an arbitrary point; the offsets
   the code uses run straight through the cut, so it is one array. */
extern u_short g_btl_pack_sectors[];
extern int     g_btl_voice_base;

#define BTL_VOICE_FIRST 0x100

/* Where a voice bank is read to, and the three words it opens with - the VAB
   body, the VAB header and the SEQ, the same head every bank on this disc
   has. One sub-sequence each. */
#define BTL_VOICE_BUFFER 0x801A1200
#define BTL_VOICE_SEPS   1

#define g_btl_voice_vb  (*(u_char **)0x801A1200)
#define g_btl_voice_vh  (*(u_char **)0x801A1204)
#define g_btl_voice_seq (*(u_long **)0x801A1208)

extern void BtlLoadVoiceBank(int entry);
extern void BtlReadVoiceBank(int entry);
extern void BtlOpenVoiceBank(void);
extern void BtlWaitVabTrans(void);

extern short  g_btl_vab[];
extern short  g_btl_seq[];
extern u_char g_btl_seq_count[];

/* Reading the demon voices in. Two places do it the same way - the
   negotiation as it opens and the round as it starts - so the run of buffers
   they read into, and the slots they walk, are here rather than in both.

   Every bank is the same size, so the destination simply walks. The slots
   below are the ones that belong to nobody in the party: the odd ones from
   ten, which is what makes the walk step by two. */
#define BTL_VOICE_BANKS      ((u_long *)0x80172400)
#define BTL_VOICE_BANK_BYTES 0x5DC0
#define BTL_VOICE_SLOT_FIRST 10
#define BTL_VOICE_SLOT_LAST  15
#define BTL_VOICE_SLOT_STEP  2

extern void BtlLoadSound(u_long *dest, int entry);
extern void BtlLoadSlotSound(u_long *dest, int slot);

/* Who each of the sixteen slots was opened for, or -1 while it is free. */
extern short g_btl_slot_owner[];

/* Raised once the run above has been read, so the round does not read it
   again on top of a negotiation that already did. */
extern int g_btl_talk_sounds_loaded;

/* The two sets of banks a fighter's voice can come from: the party's own,
   indexed by the record key, and the slots the rest of the field uses. */
extern BtlSoundBank g_btl_banks[];
extern BtlSoundBank g_btl_slot_banks[];

extern int  BtlSoundOpen(const BtlSoundBank *banks, int slot, int index);
extern void BtlSoundClose(int slot);

/* The two marks the battle's opening sequence carries. The first hands the
   fanfare over to the second sub-sequence and puts the hit under it; the
   second is the end, and is the only thing that raises g_btl_intro_bgm_done -
   unless the caller finds the sequence already over, in which case it raises
   the flag itself rather than arming the callback at all. */
#define BTL_BGM_MARK_NEXT 0x6B
#define BTL_BGM_MARK_END  0x6C

extern u_char g_btl_intro_bgm_done;
extern void   BtlIntroBgmMark(short seq, short sep, short mark);

/* Callers pass the sequence as a word. BtlSePlay narrows it for libsnd. */
extern void BtlSePlay(int slot, int seq);

extern int g_btl_bgm_state;
extern int g_btl_bgm_seq;
extern u_char g_btl_bgm_table[][4];
extern void BtlBgmOpen(void);
extern void BtlBgmChange(int track, int column, int base);

#endif
