/* Persona 1 (JP) - five copies of one effect stacked on a fighter.  BTLP only.
 *   0x800B9D58 BtlOpenFxStack
 *
 * What move 0x31's and move 0x32's start handlers both build. Five records on
 * the fighter in the given slot, each two frames later than the one above it
 * and each carrying its place in the stack as its mark, chained downward so
 * the bottom one - the one that arrives first - is the head and the one
 * answered.
 *
 * The four above the bottom are drawn from upload slot one rather than the
 * effect's own: the page the effect was staged into is copied there with both
 * blend bits set, so the copies are laid over the field at the full rate
 * whatever the staged page was left holding. The record's own +0xCD is what
 * says which of the two pages it is drawn from.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* Records in the stack, and how much later each one above the bottom
   arrives. */
#define FX_STACK       5
#define FX_STACK_STEP  2

/* The upload slot the effect is staged into, the one the copies are drawn
   from, and the two page bits that say it is blended at the full rate. */
#define FX_TPAGE_SLOT  29
#define FX_STACK_PAGE  1
#define TPAGE_ABR      0x60

BtlObj *BtlOpenFxStack(int slot)
{
    BtlObj *o;
    BtlObj *after;
    int     i;

    i = FX_STACK - 1;
    after = 0;
    do {
        o = BtlOpenFxObj2(slot, i * FX_STACK_STEP);
        o->attached = after;
        o->mark_num = i;
        if (i != 0) {
            g_btl_tpage[FX_STACK_PAGE] = g_btl_tpage[FX_TPAGE_SLOT] | TPAGE_ABR;
            o->unkCD = FX_STACK_PAGE;
        }
        i--;
        after = o;
    } while (i >= 0);
    return o;
}
