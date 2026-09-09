/* Persona 1 (JP) - walking the fighters' colours.  BTLP only.
 *   0x8008A20C BtlStepCluts
 *
 * Every actor has a palette that is drawn from and a second one it is meant to
 * reach, and this moves the first a step toward the second once a frame - a
 * tint fading in, or an ailment's colour draining away.
 *
 * A step is one unit on each of the three five-bit channels, so a colour takes
 * as many frames as its furthest channel has left. Both palettes go into the
 * scratchpad first, which is where the walk is done, and the result is written
 * back over the drawn one.
 *
 * g_btl_clut_fading says who still has work. The bit is cleared as the actor is
 * picked up and put back by any channel that moved, so it falls away by itself
 * on the frame the two palettes agree and the actor is skipped from then on.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>

/* All fourteen, party and enemies alike. */
#define BTL_ACTORS 14

/* Bytes between one actor's palette and the next. */
#define BTL_CLUT_STRIDE 0x200

/* Where the walk is done: two palettes' worth of scratchpad. */
#define CLUT_NOW ((u_short *)0x1F800000)
#define CLUT_TO  ((u_short *)0x1F800200)

/* A 15-bit colour, and one step on each channel. */
#define CLUT_R      0x001F
#define CLUT_G      0x03E0
#define CLUT_B      0x7C00
#define CLUT_R_STEP 0x0001
#define CLUT_G_STEP 0x0020
#define CLUT_B_STEP 0x0400
#define CLUT_STP    0x8000

extern u_short  g_btl_clut_fading;
extern u_short *g_btl_actor_clut;
extern u_short *g_btl_actor_clut_to;

void BtlStepCluts(void)
{
    u_short *now;
    u_short *to;
    u_char  *src;
    u_char  *dst;
    u_short  mask;
    int      i;
    int      j;
    int      n;
    int      r;
    int      g;
    int      b;
    int      want;
    int      out;
    /* Declared and never read; the original reserves the room. */
    int      spare[2];

    i = 0;
    mask = 1;
    src = (u_char *)g_btl_actor_clut;
    dst = (u_char *)g_btl_actor_clut_to;
    do {
        if ((mask & g_btl_clut_fading) != 0) {
            now = CLUT_NOW;
            to = CLUT_TO;
            g_btl_clut_fading &= ~mask;
            n = g_btl_actors[i].clut_len;
            memcpy((u_char *)CLUT_NOW, src, n * 2);
            memcpy((u_char *)CLUT_TO, dst, n * 2);
            j = 0;
            if (n != 0) {
                do {
                    if (*now != *to) {
                        out = CLUT_STP;
                        r = *now & CLUT_R;
                        want = *to & CLUT_R;
                        if (r != want) {
                            g_btl_clut_fading |= mask;
                            if (r < want) {
                                r += CLUT_R_STEP;
                            } else {
                                r -= CLUT_R_STEP;
                            }
                        }
                        out |= r;
                        g = *now & CLUT_G;
                        want = *to & CLUT_G;
                        if (g != want) {
                            g_btl_clut_fading |= mask;
                            if (g < want) {
                                g += CLUT_G_STEP;
                            } else {
                                g -= CLUT_G_STEP;
                            }
                        }
                        out |= g;
                        b = *now & CLUT_B;
                        want = *to & CLUT_B;
                        if (b != want) {
                            g_btl_clut_fading |= mask;
                            if (b < want) {
                                b += CLUT_B_STEP;
                            } else {
                                b -= CLUT_B_STEP;
                            }
                        }
                        *now = out | b;
                    }
                    j++;
                    now++;
                    to++;
                } while (j < n);
            }
            memcpy(src, (u_char *)now - n * 2, n * 2);
        }
        i++;
        mask <<= 1;
        src += BTL_CLUT_STRIDE;
        dst += BTL_CLUT_STRIDE;
    } while (i < BTL_ACTORS);
}
