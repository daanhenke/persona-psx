/* Persona 1 (JP) - shaking the view.  ADV only.
 *   0x800AE174 ViewShakeStop   0x800AE1A4 ViewShakeStep
 *
 * A scene shakes the room by setting g_view_shake to 7, 8 or 9; every frame
 * AdvRunFrame calls ViewShakeStep, which throws the view a random offset of
 * up to 1, 4 or 7 pixels each way. The renderer adds the offsets to the
 * camera, and the second pair to the map layer's scroll.
 */
#include <decomp/types.h>
#include <stdlib.h>

#define SHAKE_SMALL  7
#define SHAKE_MEDIUM 8
#define SHAKE_LARGE  9

extern u_short g_view_shake;
extern u_short g_view_dx;
extern u_short g_view_dy;
extern u_short g_view2_dx;

/* Stops the shake and puts the view back. */
void ViewShakeStop(void)
{
    g_view_shake = 0;
    g_view_dy = 0;
    g_view_dx = 0;
    g_view_dy = 0;
    g_view2_dx = 0;
}

void ViewShakeStep(void)
{
    switch (g_view_shake) {
    case SHAKE_SMALL:
        g_view_dy = rand() % 2;
        g_view_dx = rand() % 2;
        break;
    case SHAKE_MEDIUM:
        g_view_dy = rand() % 5;
        g_view_dx = rand() % 5;
        break;
    case SHAKE_LARGE:
        g_view_dy = rand() % 8;
        g_view_dx = rand() % 8;
        break;
    }
}
