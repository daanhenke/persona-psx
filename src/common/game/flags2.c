/* Persona 1 (JP) - collecting a bank-2 group, and the getter it uses.
 *
 * A static copy of the routine in flags.c, compiled into other units and so
 * present at its own address:
 *   DNG 0x800991D0   ADV 0x80098A08   S2D 0x80089680
 *
 * The banks are in the shared save-game work area, so one source covers every
 * overlay. See flags.c for the layout of the three banks. The event-flag copy
 * is a unit of its own well ahead of this, in eventflag2.c, and the bank-3
 * setter one behind it in flagbank3.c.
 */
#include <decomp/types.h>

#define g_flags_bank2 ((u_char *)0x801F2A48)

/* Where the collected flags are left. Reached by hardcoded address, and S2D's
   copy sits 0x20000 higher like the rest of its work area. */
#define g_items_pending ((u_char *)(0x800EAE4C + WORK_BIAS))

/* Two bytes per group: the first flag of the run and how many follow. This one
   is in each overlay's own data, so it goes through the linker symbol. */
extern const u_char g_flag_groups[];

/* Every set flag of one bank-2 group, gathered into g_items_pending, and how
   many there were.

   The two range bytes are held as separate pointers and re-read on every pass
   rather than being summed once into a local, which is what the reload in the
   loop tail is. FlagBank2Get is defined further down this file, so the call
   here has no prototype and the result is narrowed by the char it lands in. */
u_char FlagsCollectGroup(u_char group)
{
    u_char flag;
    u_char n;
    char set;

    n = 0;
    flag = g_flag_groups[group * 2];
    if (flag < g_flag_groups[group * 2] + g_flag_groups[group * 2 + 1]) {
        do {
            set = FlagBank2Get(flag);
            if (set) {
                g_items_pending[n] = flag;
                n++;
            }
            flag++;
        } while (flag
                 < g_flag_groups[group * 2] + g_flag_groups[group * 2 + 1]);
    }
    return n;
}

int FlagBank2Get(short id)
{
    u_char *p;
    int     v;

    p = g_flags_bank2;
    v = p[id / 8];
    v = v & (1 << (id & 7));
    return v;
}
