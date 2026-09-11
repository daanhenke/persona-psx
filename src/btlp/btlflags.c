/* Persona 1 (JP) - the battle's own view of the story flags.  BTLP only.
 *   0x800976F4 BtlEventFlagTest   0x80097730 BtlEventFlagSet
 *   0x80097774 BtlEventFlagClear
 *
 * The same bank src/common/game/flags.c reaches a byte at a time, reached
 * here a word at a time: the id divides by 32 rather than by 8 and the bit is
 * shifted into a whole word. Both spellings land on the same bits, and both
 * answer with the masked bit rather than 0 or 1.
 *
 * The id is signed, so the division carries the bias gcc adds for a negative
 * numerator; nothing passes one.
 *
 * The test folds the bank's address into its single access and the other two
 * materialise it into a register, which is not a difference in the source -
 * it is only that a read-modify-write needs the address twice.
 *
 * The test scores 99.33% rather than 100% for the same reason
 * src/common/game/eventflag.c does: the bank is reached by address, so the
 * build emits no relocation where the disassembly invented a symbol. The two
 * words are identical - `addu $at,$a0,$at` is the literal form, and a linker
 * symbol would have assembled the operands the other way round.
 */
#include <decomp/types.h>

/* The story flags, by address - the overlay does not link against a symbol
   for them. */
#define g_btl_event_flags ((u_long *)0x801F29C8)

#define BTL_FLAG_BITS 32


int BtlEventFlagTest(int id)
{
    return g_btl_event_flags[id / BTL_FLAG_BITS] & (1 << (id % BTL_FLAG_BITS));
}

void BtlEventFlagSet(int id)
{
    g_btl_event_flags[id / BTL_FLAG_BITS] |= 1 << (id % BTL_FLAG_BITS);
}

void BtlEventFlagClear(int id)
{
    g_btl_event_flags[id / BTL_FLAG_BITS] &= ~(1 << (id % BTL_FLAG_BITS));
}
