/* Integrate a timed field rotation and restore directional input. */
#include <persona/s2d/s2d.h>
extern short D_800B0D3C, D_800B0D40, D_800B0D38;
extern int g_key_lock;

void func_8008BA9C(void)
{
    if (D_800B0D3C > 0) {
        D_800B0D3C--;
        g_key_lock = 0;
        g_map_xform.rot.vy += D_800B0D40 * 128;
        if (D_800B0D3C <= 0) {
            g_key_lock = 1;
            g_s2d_facing = D_800B0D38;
        }
    }
}
