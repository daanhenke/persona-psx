/* Persona 1 (JP) - the layer in front of the battle.  BTLP only.
 *   0x80065E74 BtlDrawFront
 *   0x80065FB4 BtlFrontStepReady
 *   0x80065FBC BtlFrontStepDraw
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

int BtlDrawFront(u_long *ot)
{
    short step;

    step = g_btl_front_step;
    if (step != 0) {
        switch (step) {
        case 1:
            break;
        case 2:
            g_btl_front_timer--;
            if (g_btl_front_timer == -1) {
                g_btl_front_step = step + 1;
            }
            break;
        case 3:
            if (BtlFrontStepReady() == 0) {
                g_btl_front_step++;
            }
            break;
        case 4:
            break;
        default:
            g_btl_front_step = 0;
            g_btl_front_flag = 0;
            break;
        }
        BtlFrontStepDraw(ot);
    }
    BtlDrawPanel(g_btl_front_side, ot);
    BtlFaceDraw(g_btl_front_side, ot);
    BtlDrawUi(ot);
    BtlPadRepeat();
    g_btl_front_side ^= 1;
    return g_btl_front_step;
}

int BtlFrontStepReady(void)
{
    return 0;
}

void BtlFrontStepDraw(u_long *ot)
{
}

