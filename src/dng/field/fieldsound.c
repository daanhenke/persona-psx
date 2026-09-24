/* Persona 1 (JP) - the field's sequences, and leaving for a battle.
 * DNG only.
 *   0x80070A94 FieldStartBattle
 *   0x80070BF0 FieldPlayJingle
 *   0x80070C90 FieldPlaySeq
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <rand.h>
#include <persona/main/state.h>
#include <persona/dng/field.h>

/* The field's sequence data, loaded at SEQ_BASE with a table of offsets at
   its head; an offset of 0 is a sequence the floor does not have. */
#define SEQ_BASE    0x801CD000
#define SEQ_OFFSETS ((u_long *)SEQ_BASE)

/* The handle a jingle plays in, and the battle's fanfare. */
#define SEQ_JINGLE  17
#define SEQ_BATTLE  18

/* Hands the field over to a battle: one of two transitions, chosen at
   random, the battle's field preloaded meanwhile, every sequence but the
   fanfare closed and the fanfare started fading in. */
void FieldStartBattle(void)
{
    int i;
    int kind;

    kind = rand() % 3;
    if (kind == 2) {
        func_80071EF8();
    } else {
        FieldFxBegin();
    }
    PreloadBtlField();
    for (i = 0; i < FIELD_SEQS; i++) {
        if (i != SEQ_BATTLE && g_seq_handles[i] != -1) {
            SsSetNck(g_seq_handles[i]);
        }
    }
    SsVabClose(g_vab_handles[0]);
    if (g_vab_handles[2] != -1) {
        SsVabClose(g_vab_handles[2]);
    }
    SsSeqPlay(g_seq_handles[SEQ_BATTLE], 1, 0);
    SsSeqSetDecrescendo(g_seq_handles[SEQ_BATTLE], 0x7F, 0xF0);
    if (kind == 2) {
        func_80072018();
    } else {
        FieldFxRun(kind);
    }
    g_field_lit = 0;
    g_state_next = GAME_STATE_BTL;
}

/* Plays sequence `n` as a jingle, replacing whatever jingle is playing, for
   `loops` repeats. */
void FieldPlayJingle(u_char n, short loops)
{
    if (SEQ_OFFSETS[n] != 0) {
        SsSetNck(g_seq_handles[SEQ_JINGLE]);
        g_seq_handles[SEQ_JINGLE] = SsSeqOpen((u_long *)(SEQ_BASE + SEQ_OFFSETS[n]), g_vab_handles[2]);
        SsSeqPlay(g_seq_handles[SEQ_JINGLE], 1, loops);
    }
}

/* Plays sequence `n` in its own handle, reopening it on the VAB its number
   belongs to: the first three on the first VAB, the next fifteen on the
   second, the rest on the third. Old-style, like FieldInitSprite: callers
   pass ints, and n is narrowed again after the call to SsSetNck. */
void FieldPlaySeq(n, loops)
    u_char n;
    short  loops;
{
    int vab;

    if (SEQ_OFFSETS[n] != 0) {
        if (g_seq_handles[g_seq_slot[n]] != -1) {
            SsSetNck(g_seq_handles[g_seq_slot[n]]);
        }
        if (n >= 18) {
            vab = 2;
        } else {
            vab = n >= 3;
        }
        g_seq_handles[g_seq_slot[n]] = SsSeqOpen((u_long *)(SEQ_BASE + SEQ_OFFSETS[n]), g_vab_handles[vab]);
        SsSeqPlay(g_seq_handles[g_seq_slot[n]], 1, loops);
    }
}
