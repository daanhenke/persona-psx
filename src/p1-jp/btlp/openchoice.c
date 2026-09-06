/* Persona 1 (JP) - showing the choice the battle has arrived at.  BTLP only.
 *   0x80069BC8 BtlOpenChoice
 *
 * The rows live in whatever was loaded into the scratch area, twelve bytes
 * each, reached through the offset the header keeps. Two frames are drawn
 * before the box opens so the talkers have settled and the box comes up over
 * a clean screen rather than mid-animation.
 */
#include <types.h>

/* One row of the choice table. */
#define CHOICE_ROW 12

/* The scratch area is reached by address, the way the rest of the work area
   is. */
#define g_btl_scratch      ((u_char *)0x801C0000)
#define g_btl_choice_table (*(int *)0x801C000C)

extern u_short g_btl_choice_row;
extern int     g_btl_choice_shown;

extern void BtlSeqRun(void);
extern void BtlEndTalking(void);
extern void BtlDrawFrame(void);
extern void BtlMenuOpenChoices(u_short *row);
extern void BtlIndicatorIcon(void);

void BtlOpenChoice(void)
{
    BtlSeqRun();
    BtlEndTalking();
    g_btl_choice_shown = g_btl_choice_row;
    BtlDrawFrame();
    BtlDrawFrame();
    BtlMenuOpenChoices((u_short *)(g_btl_scratch + g_btl_choice_table
                                   + g_btl_choice_row * CHOICE_ROW));
    BtlIndicatorIcon();
}
