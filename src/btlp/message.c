/* Persona 1 (JP) - finding a message.  BTLP only.
 *   0x80074274 BtlMessage
 *
 * The pack BtlLoadScratch reads into the scratch buffer holds the battle's
 * message scripts. Its first word is the offset to a directory, and every
 * directory entry is itself an offset from the directory's own address, so
 * nothing in the pack has to be fixed up when it lands.
 *
 * Which directory slot a message uses comes from a second table, fourteen
 * entries to a group, sitting a little way past the end of the loaded pack -
 * so a group is one situation's worth of messages and the index picks one of
 * them. What comes back is a sequencer script: every caller hands it to
 * BtlSeqPlay.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>

/* The (group, index) table, past the end of whatever was loaded. */
#define BTL_MSG_TABLE 0x3B4
#define BTL_MSG_GROUP 14

/* The buffer is reached by its address, like loadscratch.c. Through the linker
   symbol it costs a word more: 0x801C0000 has nothing in its low half, so a
   literal is one `lui` and every use folds into a displacement, where a
   relocated symbol needs the `addiu` as well. The directory offset is added
   twice over rather than being kept as a pointer.

   What is below is the right nineteen instructions in the right order - the
   load-delay nop included, which is what the split return is for - but gcc
   puts the buffer in v1 and the directory offset in a0 where the original has
   them the other way round. Reordering the two loads, the two statements and
   the declarations all leave that alone. */
#define g_btl_scratch ((u_char *)0x801C0000)
extern u_char *g_btl_scratch_end;

#ifdef NON_MATCHING
u_char *BtlMessage(int index, int group)
{
    u_char *at;
    u_long  dir;
    u_short slot;

    slot = ((u_short *)(g_btl_scratch_end + BTL_MSG_TABLE))
           [group * BTL_MSG_GROUP + index];
    dir = *(u_long *)g_btl_scratch;
    at = *(u_long *)(g_btl_scratch + dir + slot * 4) + g_btl_scratch;
    return at + dir;
}
#else
INCLUDE_ASM("btlp/nonmatchings/message", BtlMessage);
#endif

