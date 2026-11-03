/* Persona 1 (JP) - advance the capped play-time counter. */
#include <decomp/types.h>

extern u_char minute __asm__("g_playtime_min");
extern u_char second __asm__("g_playtime_min+0x1");
extern u_char frame __asm__("g_playtime_frame");

void func_80097288(int delta)
{
    u_char *time = (u_char *)0x801F29BC;
    u_int hours = time[0];
    u_int frames = frame;
    frames += delta;

    if (frames >= 60) {
        (second)++;
        if (second >= 60) {
            (minute)++;
            if (minute >= 60) {
                hours++;
                if (hours >= 101) {
                    hours = 100;
                }
                if (hours == 100) {
                    minute = 59;
                    second = 59;
                    hours = 99;
                    time[0] = hours;
                    frame = 59;
                    return;
                }
                minute = 0;
            }
            time[2] = 0;
        }
        frames = 0;
    }
    time[0] = hours;
    time[3] = frames;
}
