/* Persona 1 (JP) - forgetting which talk lines a demon has used.  BTLP only.
 *   0x800682BC BtlClearLineHistory
 *
 * A demon draws its negotiation line at random but does not repeat itself:
 * the picker builds the list of the five lines absent from that demon's row
 * of g_btl_line_used and chooses among those, writing the choice back. Once
 * the last free slot is taken the demon's g_btl_line_cycle entry drops to
 * zero and it stops drawing fresh lines.
 *
 * This puts both back to -1 for all four demons, and runs from the battle
 * reset beside BtlClearMemberLines, which does the same for the party.
 */
#include <types.h>

/* Demons that can be in one offer. */
#define BTL_LINE_DEMONS 4

/* Talk lines each of them has. */
#define BTL_LINES 5

extern short g_btl_line_cycle[];
extern short g_btl_line_used[];

void BtlClearLineHistory(void)
{
    short *cycle;
    short *used;
    short *slot;
    short  none;
    int    demon;
    int    i;

    /* The sentinel is written to a variable of its own before either pointer:
       that order is what the original has, and inlining it costs the match. */
    demon = 0;
    none = -1;
    used = g_btl_line_used;
    cycle = g_btl_line_cycle;
    do {
        *cycle = none;
        i = BTL_LINES - 1;
        slot = used + BTL_LINES - 1;
        for (; i >= 0; i--) {
            *slot-- = none;
        }
        used += BTL_LINES;
        demon++;
        cycle++;
    } while (demon < BTL_LINE_DEMONS);
}
