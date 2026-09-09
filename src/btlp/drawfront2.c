/* Persona 1 (JP) - the layer in front of the battle.  BTLP only.
 *   0x80065E74 BtlDrawFront
 *   0x80065FB4 BtlFrontStepReady
 *   0x80065FBC BtlFrontStepDraw
 *   0x8006B6A8 BtlDrawBehind
 *
 * Once a frame, with the ordering table the caller is filling: the pop-up
 * panel, the portrait of whoever is acting, the whole 2D layer, and the pad's
 * repeat tick. g_btl_front_side flips every frame and is the side the panel
 * and the portrait are drawn from.
 *
 * The switch is an animation that is not in the shipped build. Nothing outside
 * this routine writes g_btl_front_step, so it never leaves zero and the switch
 * falls straight through to the drawing. What it would have run is a countdown
 * on step 2, a wait on BtlFrontStepReady on step 3, and BtlFrontStepDraw on
 * every step it was going - and both of those are empty.
 */
#include <decomp/types.h>
#include <persona/btlp/input.h>

extern short g_btl_front_step;
extern short g_btl_front_timer;
extern int   g_btl_front_side;
extern int   g_btl_front_flag;

extern void BtlDrawPanel(int side, u_long *ot);
extern void BtlFaceDraw(int side, u_long *ot);
extern void BtlDrawUi(u_long *ot);

/* The two halves of the animation that is not in the shipped build. One says
   it is never ready, the other draws nothing. */
/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */
extern int BtlDrawFront(u_long *ot);
extern int BtlFrontStepReady(void);
extern void BtlFrontStepDraw(u_long *ot);

void BtlDrawBehind(u_long *ot)
{
}
