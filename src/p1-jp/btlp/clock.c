/* Persona 1 (JP) - the battle's VSync callback.  BTLP only.
 *   0x8008D7B0 BtlClockTick
 *
 * Installed by the overlay's entry point, so it runs once a field for as long
 * as the battle lasts whatever else the game is doing. Two jobs.
 *
 * The first is the save file's play-time clock: four bytes, fields into
 * seconds into minutes into hours. At 99 hours it stops rather than wrapping,
 * and everything below is pinned at 59 with it, so a full clock reads
 * 99:59:59 and stays there.
 *
 * The second is pacing. Raising g_btl_frame_due is what lets the frame drawer
 * go, so the battle runs off the field interrupt rather than off however long
 * its own drawing took, and g_btl_frame_queued stops a second frame being
 * asked for before the first has been taken.
 */
#include <types.h>

/* Fields to the second, and the same count for seconds and minutes. */
#define CLOCK_WRAP 60

/* Where the clock stops. */
#define CLOCK_MAX_HOURS 99

/* The hours are reached by address and the rest by name: the original keeps
   one register pointing at the hours across the whole callback, which is what
   a literal does and a symbol does not. */
#define g_playtime (*(u_char *)0x801F29BC)

extern u_char g_playtime_min;
extern u_char g_playtime_sec;
extern u_char g_playtime_frame;

extern u_char g_btl_frame_due;
extern u_char g_btl_vsync_count;
extern int    g_btl_frame_queued;
extern int    g_btl_frame_ticks;

void BtlClockTick(void)
{
    g_btl_vsync_count++;
    g_btl_frame_ticks++;
    if (++g_playtime_frame >= CLOCK_WRAP) {
        g_playtime_frame = 0;
        if (++g_playtime_sec >= CLOCK_WRAP) {
            g_playtime_sec = 0;
            if (++g_playtime_min >= CLOCK_WRAP) {
                g_playtime_min = 0;
                g_playtime++;
                if (g_playtime > CLOCK_MAX_HOURS) {
                    g_playtime = CLOCK_MAX_HOURS;
                    g_playtime_min = CLOCK_WRAP - 1;
                    g_playtime_sec = CLOCK_WRAP - 1;
                    g_playtime_frame = CLOCK_WRAP - 1;
                }
            }
        }
    }
    if (g_btl_frame_queued == 0 && g_btl_frame_ticks >= 0) {
        g_btl_frame_due = 1;
        g_btl_frame_queued = 1;
    }
}
