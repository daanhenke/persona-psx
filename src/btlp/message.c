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

/* The (group, index) table, past the end of whatever was loaded. */
#define BTL_MSG_TABLE 0x3B4
#define BTL_MSG_GROUP 14

/* The buffer is reached by its address, like loadscratch.c. Through the linker
   symbol it costs a word more: 0x801C0000 has nothing in its low half, so a
   literal is one `lui` and every use folds into a displacement, where a
   relocated symbol needs the `addiu` as well. The directory offset is added
   twice over rather than being kept as a pointer.

   The table's byte offset is worked out into a variable of its own before it
   meets the pointer. Indexed in place, gcc adds the index to the pointer
   rather than the pointer to the index, and the buffer and the directory
   offset swap registers. */
#define g_btl_scratch ((u_char *)0x801C0000)
extern u_char *g_btl_scratch_end;

u_char *BtlMessage(int index, int group)
{
    u_char *at;
    u_long  dir;
    u_short slot;
    int     i;

    i = (group * BTL_MSG_GROUP + index) * 2;
    slot = *(u_short *)(g_btl_scratch_end + i + BTL_MSG_TABLE);
    dir = *(u_long *)g_btl_scratch;
    at = *(u_long *)(g_btl_scratch + dir + slot * 4) + g_btl_scratch;
    return at + dir;
}
