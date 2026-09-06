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
#include <types.h>

/* The (group, index) table, past the end of whatever was loaded. */
#define BTL_MSG_TABLE 0x3B4
#define BTL_MSG_GROUP 14

/* Unlike loadscratch.c, this file reaches the buffer through the linker symbol
   rather than by its address; the directory offset is added twice over rather
   than being kept as a pointer. */
extern u_char  g_btl_scratch[];
extern u_char *g_btl_scratch_end;

u_char *BtlMessage(int index, int group)
{
    u_long  dir;
    u_short slot;

    slot = *(u_short *)(g_btl_scratch_end +
                        (group * BTL_MSG_GROUP + index) * 2 + BTL_MSG_TABLE);
    dir = *(u_long *)g_btl_scratch;
    return g_btl_scratch + dir +
           *(u_long *)(g_btl_scratch + dir + slot * 4);
}
