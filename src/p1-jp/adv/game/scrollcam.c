/* Persona 1 (JP) - the scripted camera pan.  ADV @ 0x800AEDF0.
 *
 * An event script asks for a direction, a number of tiles and a speed, and the
 * camera walks there with the same WalkAdvance the actors use: sixteen phases
 * to a tile, advanced speed + 1 at a time. A frame is drawn after every step,
 * so the call blocks until the pan is over - which is what makes the camera
 * move an event beat rather than something the player can interrupt.
 */
#include <types.h>

extern short g_cam_x;
extern short g_cam_y;

#define WALK_PHASES 0x10

extern void WalkAdvance(short *y, short *x, u_char dir, u_char phase,
                        u_char step);
extern void AdvRunFrame(void);

void AdvScrollCamera(u_char dir, char tiles, char speed)
{
    u_char phase;
    u_char step;

    step = speed + 1;
    if (tiles != 0) {
        do {
            phase = 0;
            do {
                WalkAdvance(&g_cam_y, &g_cam_x, dir, phase, step);
                AdvRunFrame();
                phase = phase + step;
            } while (phase < WALK_PHASES);
            tiles--;
        } while (tiles != 0);
    }
}
